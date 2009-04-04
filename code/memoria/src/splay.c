/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 3. April 2009
 */

/* Splay Tree backend
 * Implements a backend for the mem_list_t interface. It uses the binary
 * search tree "splay tree" which is a simple binary search tree but
 * always puts the accessed element with rotations to the top. This way
 * the tree is kept semi balanced.
 */


#include "memoria/memoria.h"

#include <stdlib.h>
#include <string.h>


/* Compares two nodes. */
static inline signed int mem_stree_comp(mem_list_t *tree, mem_node_t *comparison, mem_node_t *original) {
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

/* Core of a splay-tree.
 * We take \node as a root of a subtree in \tree. We search for \key and \klen.
 * We simply check whether the root is the key. If it is, we return it, if it is not, we do a left/right
 * rotation depending on whether the key of the root is greater or smaller than \key.
 * Then we continue at the new root. If it is a loop, we simply return.
 * Therefore, the returned element is either the searched element put to the root or one of both nearest
 * elements put to the root.
 */
static mem_node_t *mem_stree_splay(mem_list_t *tree, mem_node_t *node, mem_node_t *search) {
    mem_node_t *left, *right, *yank, tmp;
    signed int comp;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(node != NULL);

    tmp.left = NULL;
    tmp.right = NULL;
    left = &tmp;
    right = &tmp;

    while(1) {
        comp = mem_stree_comp(tree, node, search);
        if(comp < 0) {
            /* Right rotation.
             * Check next link first to prevent a loop
             * rotation.
             */
            if(!node->left) break;
            comp = mem_stree_comp(tree, node->left, search);
            if(comp < 0) {
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
        else if(comp > 0) {
            /* Left rotation.
             * Check next link first to prevent a loop
             * rotation.
             */
            if(!node->right) break;
            comp = mem_stree_comp(tree, node->right, search);
            if(comp > 0) {
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
        /* Or have we found the node? return. */
        else break;
    }

    left->right = node->left;
    right->left = node->right;
    node->left = tmp.right;
    node->right = tmp.left;
    return node;
}


void mem_splay_clear(mem_list_t *list) {
    mem_node_t *next, *cur;

    ONS_ASSERT(list != NULL);

    next = list->first;
    while(next) {
        cur = next;
        next = cur->next;

        if(list->delfunc) list->delfunc(cur);
        cur->next = cur->prev = NULL;
        mem_node_free(cur);
    }
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
    list->root = NULL;
}


mem_node_t *mem_splay_find(mem_list_t *tree, void *key, size_t len) {
    mem_node_t node;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(key != NULL);
    ONS_ASSERT(len != 0);

    if(tree->count == 0) return NULL;

    memset(&node, 0, sizeof(mem_node_t));
    node.key = key;
    node.len = len;

    tree->root = mem_stree_splay(tree, tree->root, &node);
    if(mem_stree_comp(tree, tree->root, &node) != 0) return NULL;
    else return tree->root;
}


mem_node_t *mem_splay_insert(mem_list_t *tree, mem_node_t *node) {
    signed int ret;

    ONS_ASSERT(tree != NULL && node != NULL && node->next == NULL && node->prev == NULL);

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
        tree->root = mem_stree_splay(tree, tree->root, node);
        ret = mem_stree_comp(tree, tree->root, node);
        if(ret < 0) {
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
        else if(ret > 0) {
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
            return tree->root;
        }
    }

    ++tree->count;
    return node;
}


void mem_splay_remove(mem_list_t *tree, mem_node_t *node) {
    mem_node_t *root;

    ONS_ASSERT(tree != NULL);
    ONS_ASSERT(node != NULL);
    ONS_ASSERT(tree->count > 0);

    tree->root = mem_stree_splay(tree, tree->root, node);

    /* The "splay" must have been successfull, otherwise, the user supplied a \node that
     * is not in \tree.
     */
    ONS_ASSERT(tree->root == node);

    if(tree->root->left) {
        /* Put an element without a right node at the top of the left subtree. */
        root = mem_stree_splay(tree, tree->root->left, node);
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

    tree->root->next = NULL;
    tree->root->prev = NULL;
    tree->root->left = NULL;
    tree->root->right = NULL;

    tree->root = root;
    --tree->count;
}

