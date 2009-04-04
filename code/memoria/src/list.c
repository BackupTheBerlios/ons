/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 4. April 2009
 */

/* Implements the list managenent.
 * Each backend is handled in an extra file, this file just
 * distributes the task to the right backend.
 */


#include "memoria/memoria.h"

#include <stdlib.h>
#include <string.h>


/* Following, the mem_blist array is filled with the information about
 * all available backends.
 * mem_blist can be extended/shrinked at runtime.
 */

mem_binfo_t mem_rbtree = {
    .size = sizeof(mem_node_t*) + offsetof(mem_node_t, parent),
    .clear = mem_rbtree_clear,
    .find = mem_rbtree_find,
    .insert = mem_rbtree_insert,
    .remove = mem_rbtree_remove
};

mem_binfo_t mem_splay = {
    .size = sizeof(mem_node_t*) + offsetof(mem_node_t, left),
    .clear = mem_splay_clear,
    .find = mem_splay_find,
    .insert = mem_splay_insert,
    .remove = mem_splay_remove
};

mem_binfo_t mem_table = {
    .size = sizeof(mem_hash_t*) + offsetof(mem_node_t, hash),
    .clear = mem_table_clear,
    .find = mem_table_find,
    .insert = mem_table_insert,
    .remove = mem_table_remove
};

mem_binfo_t *mem_blist[] = {
    &mem_rbtree,
    &mem_splay,
    &mem_table,
    &mem_table,
    &mem_table
};


mem_node_t *mem_node_new(unsigned int type) {
    mem_node_t *node;
    size_t size;

    ONS_ASSERT(mem_valid_type(type));

    size = mem_blist[type]->size;
    node = mem_zmalloc(size);
    return node;
}


void mem_node_free(mem_node_t *node) {
    ONS_ASSERT(node != NULL);
    ONS_ASSERT(!node->next && !node->prev);
    mem_free(node->key);
    mem_free(node);
}


void mem_node_setkey(mem_node_t *node, void *key, size_t len) {
    ONS_ASSERT(node != NULL);
    ONS_ASSERT(!node->next && !node->prev);
    ONS_ASSERT(key != NULL && len != 0);
    mem_free(node->key);
    node->key = mem_dup(key, len);
    node->len = len;
}


void mem_node_setvalue(mem_node_t *node, void *data) {
    ONS_ASSERT(node != NULL);
    node->value = data;
}


mem_list_t *mem_list_new(unsigned int type) {
    mem_list_t *list;

    ONS_ASSERT(mem_valid_type(type));

    list = mem_zmalloc(sizeof(mem_list_t));
    list->type = type;
    return list;
}


void mem_list_clear(mem_list_t *list) {
    ONS_ASSERT(list != NULL);

    if(list->count == 0) return;

    mem_blist[list->type]->clear(list);
}


void mem_list_free(mem_list_t *list) {
    ONS_ASSERT(list != NULL);

    if(list->count) mem_list_clear(list);
    mem_free(list);
}


mem_node_t *mem_list_find(mem_list_t *list, void *key, size_t len) {
    ONS_ASSERT(list != NULL);
    /* \len can be zero with hash backend. */
    ONS_ASSERT(key != NULL);

    if(list->count == 0) return NULL;

    return mem_blist[list->type]->find(list, key, len);
}


mem_node_t *mem_list_insert(mem_list_t *list, mem_node_t *node) {
    ONS_ASSERT(list != NULL && node != NULL);

    return mem_blist[list->type]->insert(list, node);
}


void mem_list_remove(mem_list_t *list, mem_node_t *node) {
    ONS_ASSERT(list != NULL && node != NULL);
    ONS_ASSERT(list->count != 0);

    mem_blist[list->type]->remove(list, node);
}

