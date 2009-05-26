/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 23. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* Hash Table backend
 * Implements a backend for mem_list_t in a hash table.
 * The size of the hash table is dynamic, i.e. the table
 * is resized when entries are added/deleted.
 * It is also possible to force the table to be static
 * with an extra function which should only be used on
 * mem_list_t when the hash table is the backend.
 */

/* Hash Table Details
 * Our hash table is based on the mem_hash() function which returns a 32bit integer. All functions
 * of this hash table work like on a static hash table except mem_tbl_resize() which resizes the
 * table without breaking the links.
 * Following the algorithm on the static hash table is described. The resizing algorithm is described
 * in mem_tbl_resize().
 *
 * Static Hash Table:
 * The size of the table is always a power of 2 (2^n whereas n >= 1). For performance reasons the
 * implementation avoids small powers of two.
 * We also create as many hashes as possible out of the 32bit hash. That is, if the table (array)
 * is 16 elements big, we only need 4 bits of the hash and therefore generate 8 hash values out of
 * the real 32bit hash.
 * Now we try all these hashes and if we find an empty bucket, we insert the node at this position.
 * If we find no empty bucket, we insert the node in the bucket of the last hash we used.
 * When searching for a node, we do the same and check always only the first node of the bucket and
 * if no node matches we search the whole list of the last bucket.
 * When removing a node we simply unlink it from the list in it's bucket.
 */


/* Configuration options. */
#define MEM_TBL_MIN 16 /* Minimal size of the table. Must be a power of two but at least 4. */
#define MEM_TBL_FULL /* Use more hashes to choose the bucket. */
#define MEM_TBL_SIZE_MULT 1 /* Always use X as many space as elements. */


#include "config/machine.h"
#include "memoria/memoria.h"
#include "memoria/alloc.h"
#include "memoria/array.h"

#include <stdlib.h>
#include <string.h>


/* Compares two nodes. */
static inline signed int mem_tbl_comp(mem_list_t *tree, mem_node_t *comparison, mem_node_t *original) {
    signed int res;

    if(tree->match) return tree->match(comparison, original);
    else {
        /* Default: Binary comparison of the keys.
         * If the keys are equal, the shorter key is smaller.
         */
        res = memcmp(comparison->key, original->key, (comparison->len > original->len)?original->len:comparison->len);
        if(res != 0) return res;
        if(original->len == comparison->len) return 0;
        if(original->len > comparison->len) return -1;
        return 1;
    }
}


/* Returns the hash value of a \val \len pair.
 * If \len is 0, \val is interpreted as already hashed (32bit unsigned integer).
 */
static inline mem_hash_t mem_tbl_gethash(void *val, size_t len) {
    if(len > 0) return mem_hash(val, len);
    else return *(mem_hash_t*)val;
}


/* Hashes a node if it is not already hashed. */
static inline void mem_tbl_hash(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);

    /* If \node->hash is NON-zero it is already hashed.
     * If it is zero, it might already been hashed and the hash was zero, however,
     * we simply rehash it in this case.
     */
    if(node->hash == 0) node->hash = mem_tbl_gethash(node->key, node->len);
}


/* Hash tokenizer.
 * The 32bit mem_hash_t can be used to create more than one small hash when the table is
 * small enough. If you pass NULL for \mask then a new mask is created, otherwise the next
 * mask is returned.
 */
static inline mem_hash_t mem_tbl_tokenize(mem_list_t *list, mem_hash_t mask) {
    if(!mask) return MEM_MASK_LOW(sizeof(mem_hash_t) * 8, list->size);
    else return MEM_MASK_MOV(sizeof(mem_hash_t) * 8, list->size, mask);
}


/* Used to iterate through all possible sub-hash values. */
#ifdef MEM_TBL_FULL
    #define MEM_TOK_MASK(list) mem_tbl_tokenize(list, 0)
    #define MEM_TOK_ITER(list) 1
    #define MEM_TOK_HASH(node, mask, iter) (((node)->hash & (mask)) / (iter))
    #define MEM_TOK_NEXT(list, mask) mem_tbl_tokenize((list), (mask))
    #define MEM_TOK_INC(list, iter) ((iter) * (list)->size)
#else
    #define MEM_TOK_MASK(list) mem_tbl_tokenize(list, 0)
    #define MEM_TOK_ITER(list) 0
    #define MEM_TOK_HASH(node, mask, iter) ((node)->hash & (mask))
    #define MEM_TOK_NEXT(list, mask) 0
    #define MEM_TOK_INC(list, iter) 0
#endif


/* Resizes the table to hold \size elements.
 * This function should be called on every insert/removal. It resizes the table only
 * when the unused buffer gets too big/small.
 *
 * We always keep a buffer of unused elements to prevent that the table is resized
 * too often. We also start with a size of 16 elements. Everything below this is not
 * worth to be stored in a hash table.
 * We always either double the size of the table or remove the half of it. That is,
 * when \size becomes greater as \list->size we double the table and when \size
 * becomes smaller than a fourth of \list->size then we half the table.
 *
 * Resizing a table is done by iterating through the global linked list and reinserting
 * each node at the new position.
 */
void mem_tbl_resize(mem_list_t *list, size_t size) {
    size_t new_size, mask, i;
    mem_hash_t hash;
    mem_node_t *node;

    SUNDRY_ASSERT(list != NULL);

    /* If the list is not initialized yet, we initialize it here. */
    if(!list->table) {
        /* Initialization should be done on the first INSERT. */
        SUNDRY_ASSERT(size == 1);

        if(list->type == MEM_TABLE_STATIC) new_size = (list->size >= MEM_TBL_MIN)?list->size:1024;
        else new_size = MEM_TBL_MIN * MEM_TBL_SIZE_MULT;
        list->table = mem_zmalloc(new_size * sizeof(mem_node_t*));
        list->size = new_size;
        return;
    }

    /* First check, whether we need to resize the table.
     * Resizing is done when:
     * - \size is bigger than \list->size
     * - \size is a fourth of \list->size
     * But:
     * - MEM_TABLE_STATIC is never resized.
     * - MEM_TABLE_GREEDY does never shrink in size.
     * - The table must always be greater or equal to MEM_TBL_MIN.
     */
    if(list->type == MEM_TABLE_STATIC) return;
    else if((size * MEM_TBL_SIZE_MULT) > list->size) {
        new_size = list->size * 2;
        /* integer overflow protection */
        if(new_size < list->size) return;
    }
    else if(list->type == MEM_TABLE_GREEDY) return;
    else if((size * MEM_TBL_SIZE_MULT) <= (list->size / 4)) new_size = list->size / 2;
    else return;

    if(new_size < (MEM_TBL_MIN * MEM_TBL_SIZE_MULT)) return;

    /* Allocate the new table. */
    SUNDRY_ASSERT(new_size > 0);
    mem_free(list->table);
    list->table = mem_zmalloc(sizeof(mem_node_t*) * new_size);
    list->size = new_size;

    /* Now, the real workhorse. We have to reinsert all elements.
     * The strategy is keeping the global linked list but clearing
     * all buckets. This is already done by the reallocation of
     * the table.
     */
    /* Algorithm is the same as in INSERT but we do not check whether
     * the element is already in the list because it is guaranteed
     * that the key is unique in the whole table.
     */
    node = list->first;
    while(node) {
        mask = MEM_TOK_MASK(list);
        i = MEM_TOK_ITER(list);
        do {
            hash = MEM_TOK_HASH(node, mask, i);

            if(!list->table[hash]) break;

            i = MEM_TOK_INC(list, i);
            mask = MEM_TOK_NEXT(list, mask);
        } while(mask);

        if(list->table[hash]) {
            node->right = list->table[hash]->right;
            list->table[hash]->right = node;
        }
        else {
            node->right = NULL;
            list->table[hash] = node;
        }
        node->bucket = hash;
        node = node->next;
    }
}


void mem_table_clear(mem_list_t *list) {
    mem_node_t *next, *cur;

    SUNDRY_ASSERT(list != NULL);

    next = list->first;
    while(next) {
        cur = next;
        next = cur->next;

        if(list->delfunc) list->delfunc(cur);
        cur->next = cur->prev = NULL;
        mem_node_free(cur);
    }
    list->count = 0;
    mem_free(list->table);
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
    list->table = NULL;
}


mem_node_t *mem_table_find(mem_list_t *list, void *key, size_t len) {
    size_t mask, i;
    mem_node_t search;
    mem_hash_t hash;
    mem_node_t *iter;

    SUNDRY_ASSERT(list != NULL && key != NULL);
    if(list->count == 0) return NULL;

    /* Create a temporary node that we need to search the list.
     * mem_tbl_hash() recognizes whether \key is already a hash, when
     * the user sets \len to 0.
     */
    memset(&search, 0, sizeof(mem_node_t));
    search.key = key;
    search.len = len;
    mem_tbl_hash(&search);

    /* Create as many hash values out of the 32bit hash as possible
     * and search the table for each hash. If we find a match, return it.
     */
    mask = MEM_TOK_MASK(list);
    i = MEM_TOK_ITER(list);
    do {
        hash = MEM_TOK_HASH(&search, mask, i);

        if(list->table[hash] && mem_tbl_comp(list, &search, list->table[hash]) == 0) return list->table[hash];

        i = MEM_TOK_INC(list, i);
        mask = MEM_TOK_NEXT(list, mask);
    } while(mask);

    /* We found no direct match, search the last bucket's linked list for a match. */
    if(list->table[hash]) {
        /* We already checked the first entry above, so continue with the second one. */
        iter = list->table[hash]->right;
        while(iter) {
            if(mem_tbl_comp(list, &search, iter) == 0) return iter;
            iter = iter->right;
        }
    }

    return NULL;
}


mem_node_t *mem_table_insert(mem_list_t *list, mem_node_t *node) {
    mem_hash_t bucket = 0, hash;
    unsigned int found = 0;
    mem_node_t *iter;
    size_t mask, i;

    SUNDRY_ASSERT(list && node);
    SUNDRY_ASSERT(!node->next && !node->prev && !node->right);

    /* Got sure the hash exists. */
    mem_tbl_hash(node);

    /* Resize the table if necessary. */
    mem_tbl_resize(list, list->count + 1);

    /* Create as many hash values out of the 32bit hash as possible
     * and search the table for each hash. If we find a match, return it.
     * If we find an empty bucket, safe a reference to the bucket and continue
     * searching to go sure the node is not inserted yet into the table.
     */
    mask = MEM_TOK_MASK(list);
    i = MEM_TOK_ITER(list);
    do {
        hash = MEM_TOK_HASH(node, mask, i);

        if(list->table[hash]) {
            if(mem_tbl_comp(list, node, list->table[hash]) == 0) return list->table[hash];
        }
        else if(!found) {
            found = 1;
            bucket = hash;
        }

        i = MEM_TOK_INC(list, i);
        mask = MEM_TOK_NEXT(list, mask);
    } while(mask);

    /* Go sure the node is not inserted into the list of the last bucket. */
    if(list->table[hash]) {
        /* We already checked the first entry, continue at the next one. */
        iter = list->table[hash]->right;
        while(iter) {
            if(mem_tbl_comp(list, node, iter) == 0) return iter;
            iter = iter->right;
        }
    }

    /* We know now that the node is not inserted yet.
     * If \found is 1 then we have already found an empty bucket where we can insert it,
     * otherwise we have to add it into the last bucket.
     */
    if(found) {
        list->table[bucket] = node;
        node->right = NULL;
        node->bucket = bucket;
    }
    else {
        /* Add it to the second position of the last bucket, so other search-paths
         * are not interfered.
         */
        node->right = list->table[hash]->right;
        list->table[hash]->right = node;
        node->bucket = hash;
    }

    /* Now, as last step, insert it into the global linked list of the table */
    mem_llist_nbn_push(list, node);

    ++list->count;
    return node;
}


void mem_table_remove(mem_list_t *list, mem_node_t *node) {
    mem_node_t *iter;

    SUNDRY_ASSERT(list && node);
    SUNDRY_ASSERT(list->count > 0);
    SUNDRY_ASSERT(list->table[node->bucket]);

    /* Find the previous node. */
    iter = list->table[node->bucket];
    while(iter->right) {
        if(iter->right == node) break;
        iter = iter->right;
    }
    SUNDRY_ASSERT(iter->right || list->table[node->bucket] == node);

    if(!iter->right) list->table[node->bucket] = node->right;
    else iter->right = node->right;

    /* Remove from global linked list and reset to default values. */
    node->right = NULL;
    mem_llist_nbn_remove(list, node);
    --list->count;

    /* Resize table if needed. */
    mem_tbl_resize(list, list->count);
}

