/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 23. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 23. February 2009
 */

/* Splay Tree implementation.
 * The Splay-Tree is a binary search tree which always puts the accessed
 * element to the top with left/right rotations.
 */

#include "memoria/memoria.h"
#include <string.h>


/* Compares two nodes. */
static inline ons_comp_t mem_stree_comp(mem_stree_t *tree, mem_snode_t *orig, mem_snode_t *comparison) {
    ons_comp_t comp;

    if(tree->match) return tree->match(orig, comparison);
    else {
        /* Default: Binary comparison of the keys.
         * If the keys are equal, the shorter key is smaller.
         */
        comp = memcmp(comparison->key, orig->key, (comparison->klen > orig->klen)?orig->klen:comparison->klen);
        if(comp != 0) {
            if(comp < 0) return ONS_SMALLER;
            else return ONS_GREATER;
        }
        if(orig->klen == comparison->klen) return ONS_EQUAL;
        if(orig->klen > comparison->klen) return ONS_SMALLER;
        return ONS_GREATER;
    }
}

void mem_stree_init(mem_stree_t *tree, mem_smatch_t match) {
}

bool mem_stree_add(mem_stree_t *tree, const char *key, size_t klen, void *data, mem_snode_t **result) {
    return true;
}

void *mem_stree_del(mem_stree_t *tree, mem_snode_t *node) {
    return NULL;
}

mem_snode_t *mem_stree_find(mem_stree_t *tree, const char *key, size_t klen) {
    return NULL;
}
