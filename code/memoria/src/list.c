/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. June 2009
 */

/* Implements the list managenent.
 * Each backend is handled in an extra file, this file just
 * distributes the task to the right backend.
 */


#include "config/machine.h"
#include "memoria/memoria.h"
#include "memoria/alloc.h"
#include "memoria/array.h"

#include <stdlib.h>
#include <string.h>


/* Following, the mem_blist array is filled with the information about
 * all available backends.
 * mem_blist can be extended/shrinked at runtime.
 */

mem_binfo_t mem_rbtree = {
    /* .size = */ sizeof(struct mem_node_be_rb_t) + offsetof(mem_node_t, be),
    /* .clear = */ mem_rbtree_clear,
    /* .find = */ mem_rbtree_find,
    /* .insert = */ mem_rbtree_insert,
    /* .remove = */ mem_rbtree_remove
};

mem_binfo_t mem_splay = {
    /* .size = */ sizeof(struct mem_node_be_splay_t) + offsetof(mem_node_t, be),
    /* .clear = */ mem_splay_clear,
    /* .find = */ mem_splay_find,
    /* .insert = */ mem_splay_insert,
    /* .remove = */ mem_splay_remove
};

mem_binfo_t *mem_blist[] = {
    &mem_rbtree,
    &mem_splay
};


void mem_node_init(mem_node_t *node, unsigned int type) {
    SUNDRY_ASSERT(mem_valid_type(type));
    memset(node, 0, mem_blist[type]->size);
}


void mem_node_free(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);
    SUNDRY_ASSERT(!node->next && !node->prev);
    mem_free(node->key);
    mem_free(node);
}


void mem_list_init(mem_list_t *list, unsigned int type) {
    SUNDRY_ASSERT(mem_valid_type(type));

    memset(list, 0, sizeof(mem_list_t));
    list->type = type;
}


void mem_list_clear(mem_list_t *list) {
    SUNDRY_ASSERT(list != NULL);

    if(list->count == 0) return;

    mem_blist[list->type]->clear(list);
}


void mem_list_free(mem_list_t *list) {
    SUNDRY_ASSERT(list != NULL);

    if(list->count) mem_list_clear(list);
    mem_free(list);
}


mem_node_t *mem_list_find(mem_list_t *list, void *key, size_t len) {
    SUNDRY_ASSERT(list != NULL);
    /* \len can be zero with hash backend. */
    SUNDRY_ASSERT(key != NULL);

    if(list->count == 0) return NULL;

    return mem_blist[list->type]->find(list, key, len);
}


mem_node_t *mem_list_insert(mem_list_t *list, mem_node_t *node) {
    SUNDRY_ASSERT(list != NULL && node != NULL);

    return mem_blist[list->type]->insert(list, node);
}


void mem_list_remove(mem_list_t *list, mem_node_t *node) {
    SUNDRY_ASSERT(list != NULL && node != NULL);
    SUNDRY_ASSERT(list->count != 0);

    mem_blist[list->type]->remove(list, node);
}

