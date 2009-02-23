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
    assert(tree != NULL);

    tree->root = NULL;
    tree->count = 0;
    tree->match = match;
}

void mem_stree_clean(mem_stree_t *tree) {
    assert(tree != NULL);

    while(tree->count) mem_stree_del(tree, tree->root->key, tree->root->klen);
}

bool mem_stree_add(mem_stree_t *tree, const void *key, size_t klen, void *data, void **result) {
    mem_snode_t *node;

    assert(tree != NULL);
    assert(key != NULL);
    assert(klen != 0);
    assert(data != NULL);
    assert(result != NULL);

    node = mem_malloc(sizeof(mem_snode_t) + klen + 1);
    node->key = sizeof(mem_snode_t) + (void*)node;
    node->klen = klen;
    node->data = data;
    memcpy(node->key, key, klen);

    if(!tree->root) {
        node->left = NULL;
        node->right = NULL;
        tree->root = node;
    }
    else {
        //tree->root = mem_stree_splay(tree->root, key, klen);
        switch(mem_stree_comp(tree, tree->root, node)) {
            case ONS_SMALLER:
                /* Insert before the current root. */
                node->left = tree->root->left;
                node->right = tree->root;
                tree->root->left = NULL;
                tree->root = node;
                break;
            case ONS_GREATER:
                /* Insert after the current root. */
                node->right = tree->root->right;
                node->left = tree->root;
                tree->root->right = NULL;
                tree->root = node;
                break;
            case ONS_EQUAL:
                *result = tree->root->data;
                mem_free(node);
                return false;
                break;
            default:
                assert(0);
                break;
        }
    }

    *result = node->data;
    ++tree->count;
    return true;
}

void *mem_stree_del(mem_stree_t *tree, const void *key, size_t klen) {
    assert(tree != NULL);
    assert(key != NULL);
    assert(klen != 0);

    if(tree->count == 0) return NULL;
    return NULL;
}

void *mem_stree_find(mem_stree_t *tree, const void *key, size_t klen) {
    return NULL;
}
