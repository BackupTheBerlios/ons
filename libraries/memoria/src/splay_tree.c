/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 23. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
 */

/* Splay Tree implementation.
 * The Splay-Tree is a binary search tree which always puts the accessed
 * element to the top with left/right rotations.
 */

#include "memoria/memoria.h"
#include <string.h>


/* Compares two nodes. */
static inline ons_comp_t mem_stree_comp(mem_stree_t *tree, const void *orig_key, size_t orig_len, const void *comp_key, size_t comp_len) {
    ons_comp_t comp;

    if(tree->match) return tree->match(orig_key, orig_len, comp_key, comp_len);
    else {
        /* Default: Binary comparison of the keys.
         * If the keys are equal, the shorter key is smaller.
         */
        comp = memcmp(comp_key, orig_key, (comp_len > orig_len)?orig_len:comp_len);
        if(comp != 0) {
            if(comp < 0) return ONS_SMALLER;
            else return ONS_GREATER;
        }
        if(orig_len == comp_len) return ONS_EQUAL;
        if(orig_len > comp_len) return ONS_SMALLER;
        return ONS_GREATER;
    }
}

/* Core of a splay-tree.
 * We take \node as a root of a subtree in \tree. We search for \key and \klen.
 * We simply check whether the root is the key. If it is, we return it, if it is not, we do a left/right
 * rotation depending on whether the key of the root is greater or smaller than \key.
 * Then we continue at the new root. If it is a loop, we simply return.
 * Therefore, the returned element is either the searched element put to the root or one of both nearest
 * element put to the root.
 */
static mem_snode_t *mem_stree_splay(mem_stree_t *tree, mem_snode_t *node, const void *key, size_t klen) {
    mem_snode_t *left, *right, *yank, tmp;
    ons_comp_t comp;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(node != NULL);
    ONS_ASSERT(key != NULL);
    ONS_ASSERT(klen != 0);

    tmp.left = NULL;
    tmp.right = NULL;
    left = &tmp;
    right = &tmp;

    while(1) {
        comp = mem_stree_comp(tree, node->key, node->klen, key, klen);
        if(comp <= ONS_SMALLER) {
            /* Right rotation.
             * Check next link first to prevent a loop
             * rotation.
             */
            if(!node->left) break;
            comp = mem_stree_comp(tree, node->left->key, node->left->klen, key, klen);
            if(comp <= ONS_SMALLER) {
                yank = node->left;
                node->left = yank->right;
                yank->right = node;
                node = yank;
                if(!node->left) break;
            }
            right->left = node;
            right = node;
            node = node->left;
        }
        else if(comp >= ONS_GREATER) {
            /* Left rotation.
             * Check next link first to prevent a loop
             * rotation.
             */
            if(!node->right) break;
            comp = mem_stree_comp(tree, node->left->key, node->left->klen, key, klen);
            if(comp >= ONS_GREATER) {
                yank = node->right;
                node->right = yank->left;
                yank->left = node;
                node = yank;
                if(!node->right) break;
            }
            left->right = node;
            left = node;
            node = node->right;
        }
        /* Or did we found the node? return. */
        else break;
    }

    left->right = node->left;
    right->left = node->right;
    node->left = tmp.right;
    node->right = tmp.left;
    return node;
}

void mem_stree_init(mem_stree_t *tree, mem_smatch_t match) {
    ONS_ASSERT(tree != NULL);

    tree->root = NULL;
    tree->count = 0;
    tree->match = match;
    tree->first = NULL;
    tree->last = NULL;
}

void mem_stree_clean(mem_stree_t *tree, void (*del_func)(void*)) {
    ONS_ASSERT(tree != NULL);

    if(del_func) while(tree->count) del_func(mem_stree_del(tree, tree->root->key, tree->root->klen));
    else while(tree->count) mem_stree_del(tree, tree->root->key, tree->root->klen);
}

bool mem_stree_add(mem_stree_t *tree, const void *key, size_t klen, void *data, void **result) {
    mem_snode_t *node;
    ons_comp_t ret;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(key != NULL);
    ONS_ASSERT(klen != 0);
    ONS_ASSERT(data != NULL);
    ONS_ASSERT(result != NULL);

    node = mem_malloc(sizeof(mem_snode_t) + klen + 1);
    node->key = sizeof(mem_snode_t) + (void*)node;
    node->klen = klen;
    node->data = data;
    memcpy(node->key, key, klen);

    if(!tree->root) {
        node->left = NULL;
        node->right = NULL;
        node->next = NULL;
        node->prev = NULL;
        tree->root = node;
        tree->first = node;
        tree->last = node;
    }
    else {
        tree->root = mem_stree_splay(tree, tree->root, key, klen);
        ret = mem_stree_comp(tree, tree->root->key, tree->root->klen, node->key, node->klen);
        if(ret <= ONS_SMALLER) {
            /* Insert before the current root. */
            node->left = tree->root->left;
            node->right = tree->root;
            tree->root->left = NULL;
            if(tree->root->prev) tree->root->prev->next = node;
            else tree->first = node;
            node->prev = tree->root->prev;
            node->next = tree->root;
            tree->root->prev = node;
            tree->root = node;
        }
        else if(ret >= ONS_GREATER) {
            /* Insert after the current root. */
            node->right = tree->root->right;
            node->left = tree->root;
            tree->root->right = NULL;
            if(tree->root->next) tree->root->next->prev = node;
            else tree->last = node;
            node->next = tree->root->next;
            node->prev = tree->root;
            tree->root->next = node;
            tree->root = node;
        }
        else {
            *result = tree->root->data;
            mem_free(node);
            return false;
        }
    }

    *result = node->data;
    ++tree->count;
    return true;
}

void *mem_stree_del(mem_stree_t *tree, const void *key, size_t klen) {
    mem_snode_t *root;
    void *ret;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(key != NULL);
    ONS_ASSERT(klen != 0);

    if(tree->count == 0) return NULL;

    tree->root = mem_stree_splay(tree, tree->root, key, klen);
    if(mem_stree_comp(tree, tree->root->key, tree->root->klen, key, klen) != ONS_EQUAL) return NULL;

    if(tree->root->left) {
        root = mem_stree_splay(tree, tree->root->left, key, klen);
        /* After a right rotation, we are the right child, therefore set the child
         * to one lower layer.
         */
        root->right = tree->root->right;
    }
    else {
        root = tree->root->right;
    }
    if(tree->root->prev) tree->root->prev->next = tree->root->next;
    if(tree->first == tree->root) tree->first = tree->first->next;
    if(tree->root->next) tree->root->next->prev = tree->root->prev;
    if(tree->last == tree->root) tree->last = tree->last->prev;

    ret = tree->root->data;
    free(tree->root);
    tree->root = root;
    --tree->count;

    return ret;
}

mem_snode_t *mem_stree_find(mem_stree_t *tree, const void *key, size_t klen) {
    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(key != NULL);
    ONS_ASSERT(klen != 0);

    if(tree->count == 0) return NULL;
    tree->root = mem_stree_splay(tree, tree->root, key, klen);
    if(mem_stree_comp(tree, tree->root->key, tree->root->klen, key, klen) != ONS_EQUAL) return NULL;
    else return tree->root;
}
