/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* Red-Black Tree implementation.
 * The RB-Tree is a binary search tree which is balanced by assigning
 * a binary color code to each node.
 */


#include "config/machine.h"
#include "memoria/memoria.h"
#include "memoria/alloc.h"
#include "memoria/array.h"

#include <string.h>


/* Compares two nodes. */
static signed int mem_rbt_comp(mem_list_t *list, mem_node_t *comparison, mem_node_t *original) {
    signed int res;

    if(list->match) return list->match(comparison, original);
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


/* first: Returns the most left entry in the tree.
 * last: Returns the most right entry in the tree.
 * next: Returns the next greater entry in the tree.
 * prev: Returns the next smaller entry in the tree.
 *
 * They are used to keep the *real* linked list sorted.
 * For iterating/etc. you should use the *real* linked list
 * interface instead of these functions.
 *
 * They return NULL if not found.
 */
mem_node_t *mem_rbt_first(mem_list_t *tree) {
    mem_node_t *iter;

    SUNDRY_ASSERT(tree != NULL);
    if(!tree->root) return NULL;

    iter = tree->root;
    while(iter->left) iter = iter->left;
    return iter;
}
mem_node_t *mem_rbt_last(mem_list_t *tree) {
    mem_node_t *iter;

    SUNDRY_ASSERT(tree != NULL);
    if(!tree->root) return NULL;

    iter = tree->root;
    while(iter->right) iter = iter->right;
    return iter;
}
mem_node_t *mem_rbt_next(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);

    if(node->right) {
        node = node->right;
        while(node->left) node = node->left;
        return node;
    }
    else if(!node->parent) return NULL;
    else if(node->parent->left == node) return node->parent;
    else {
        node = node->parent;
        while(node->parent && node->parent->right == node) node = node->parent;
        if(!node->parent) return NULL;
        else return node->parent;
    }
}
mem_node_t *mem_rbt_prev(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);

    if(node->left) {
        node = node->left;
        while(node->right) node = node->right;
        return node;
    }
    else if(!node->parent) return NULL;
    else if(node->parent->right == node) return node->parent;
    else {
        node = node->parent;
        while(node->parent && node->parent->left == node) node = node->parent;
        if(!node->parent) return NULL;
        else return node->parent;
    }
}


/* This inserts \node into \tree and returns \node. If \peek is true, \node is
 * not really inserted and only tried to be inserted.
 * If there is already a node in the tree with the same key as \node, \node is
 * not inserted and a pointer to the already existing node is returned.
 *
 * This does not rebalance the tree after pushing the element. You have to
 * rebalance it yourself. The new node is always colored RED, otherwise one path
 * would contain more black nodes than the others, which is forbidden.
 * Only if \node is the new root it is automatically colored BLACK.
 */
static mem_node_t *mem_rbt_push(mem_list_t *tree, mem_node_t *node, signed int peek) {
    mem_node_t *iter;
    signed int comp;

    SUNDRY_ASSERT(tree != NULL);
    SUNDRY_ASSERT(node != NULL);

    /* Algorithm:
     * If the tree is empty, add the new child as new black root node.
     * Otherwise, we go downwards the tree, compare the key with the keys of the
     * nodes and insert the node, if the desired child is empty.
     * Rebalancing is not done here.
     */

    if(!tree->root) {
        if(peek) return NULL;
        tree->root = node;
        tree->count = 1;
        tree->first = node;
        tree->last = node;

        node->color = MEM_BLACK;
        node->parent = NULL;
        node->tree = tree;
        node->left = NULL;
        node->right = NULL;
        node->prev = NULL;
        node->next = NULL;

        return node;
    }

    /* Iterate through the tree.
     * Compare \node with the node pointer to \iter. \comp contains the
     * result and we go through this list until we find an empty node.
     */

    iter = tree->root;

    next_round:

    comp = mem_rbt_comp(tree, iter, node);
    if(comp < 0) {
        if(!iter->left) {
            /* The left node is empty, we have to add the node here. */
            if(peek) return NULL;
            iter->left = node;
        }
        else {
            iter = iter->left;
            goto next_round;
        }
    }
    else if(comp > 0) {
        if(!iter->right) {
            /* The right node is empty, add \node here. */
            if(peek) return NULL;
            iter->right = node;
        }
        else {
            iter = iter->right;
            goto next_round;
        }
    }
    else {
        /* The node does already exist. Return it. */
        return iter;
    }

    /* Insert the node at the current iter-position. */
    ++tree->count;
    node->color = MEM_RED;
    node->tree = tree;
    node->parent = iter;

    node->left = NULL;
    node->right = NULL;
    node->next = mem_rbt_next(node);
    if(node->next) {
        node->prev = node->next->prev;
        node->next->prev = node;
        if(node->prev) node->prev->next = node;
        else tree->first = node;
    }
    else {
        node->prev = mem_rbt_prev(node);
        SUNDRY_ASSERT(node->prev != NULL);
        SUNDRY_ASSERT(node->prev->next == NULL);
        node->next = node->prev->next;
        node->prev->next = node;
        tree->last = node;
    }
    return node;
}


/* Rotate a node in the tree.
 * We rotate the node \node always with it's parent which causes a left
 * rotation on right nodes and vice versa.
 * Rotating the root node is simply ignored.
 */
static void mem_rbt_rotate(mem_list_t *tree, mem_node_t *node) {
    mem_node_t *parent;

    SUNDRY_ASSERT(tree != NULL);
    SUNDRY_ASSERT(node != NULL);
    if(!node->parent) return;

    parent = node->parent;

    /* Repoint the parent of the parent to the node, except if the grandparent is
     * the root node, then we simply let the root node point on the node.
     */
    if(!parent->parent) tree->root = node;
    /* Check for right rotation. */
    else if(parent->parent->left == parent) parent->parent->left = node;
    /* Must be a left rotation. */
    else parent->parent->right = node;

    /* Update the parent pointers. */
    node->parent = parent->parent;
    parent->parent = node;

    /* On right rotation, move the right child of \node to the left childs of the old parent.
     * On left rotation, vice versa.
     */
    if(parent->right == node) {
        /* left rotation */
        if(node->left) node->left->parent = parent;
        parent->right = node->left;
        node->left = parent;
    }
    else {
        /* right rotation */
        if(node->right) node->right->parent = parent;
        parent->left = node->right;
        node->right = parent;
    }
}


/* Returns the uncle of a node.
 * Returns NULL if no uncle is present.
 */
static mem_node_t *mem_rbt_uncle(mem_node_t *node) {
    if(!node->parent || !node->parent->parent) return NULL;
    if(node->parent->parent->left == node->parent)
        return node->parent->parent->right;
    else return node->parent->parent->left;
}

/* Changes the color of a node. */
#define mem_rbt_cchange(node) ((node)->color = ((node)->color == MEM_RED) ? MEM_BLACK : MEM_RED)

/* Resets a node to NULL except key/len/data. */
static void mem_rbt_reset(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);
    node->next = NULL;
    node->prev = NULL;
    node->color = MEM_RED;
    node->tree = NULL;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
}


void mem_rbtree_clear(mem_list_t *list) {
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
    list->first = NULL;
    list->last = NULL;
    list->root = NULL;
}


mem_node_t *mem_rbtree_find(mem_list_t *list, void *key, size_t len) {
    mem_node_t *node, find;

    SUNDRY_ASSERT(list != NULL);
    SUNDRY_ASSERT(key != NULL);
    SUNDRY_ASSERT(len != 0);

    memset(&find, 0, sizeof(mem_node_t));
    find.key = key;
    find.len = len;

    node = mem_rbt_push(list, &find, 1);
    return node;
}


mem_node_t *mem_rbtree_insert(mem_list_t *tree, mem_node_t *node) {
    mem_node_t *iter, *tmp;

    SUNDRY_ASSERT(tree != NULL && node != NULL && node->next == NULL && node->prev == NULL);
    SUNDRY_ASSERT(node->parent == NULL && node->tree == NULL);

    /* This key does already exist. We return the node. */
    if((tmp = mem_rbt_push(tree, node, 0)) != node) {
        return tmp;
    }

    /* Make sure the tree is balanced.
     * The new node is always red, except if it is the new root, then we simply do nothing.
     * Otherwise we adjust the new node to conform to the rules. There are three possible
     * cases which all modify the parent, thus, afterwards we need to continue checking
     * that the parent conforms to the rules.
     * The three cases are:
     * - The uncle and the parent are red: We color them black and their parent red and
     *   continue with checking their parent.
     * - Uncle is black and we are the right/left child: Rotate to the left/right and
     *   recolor parent and grandparent.
     * - The uncle is black and the parent is red: Rotate to the right/left and recolor
     *   the uncle.
     */
    iter = node;
    while(iter && iter->color != MEM_BLACK && iter->parent && iter->parent->color != MEM_BLACK) {
        tmp = mem_rbt_uncle(iter);
        if(tmp && tmp->color == MEM_RED) {
            mem_rbt_cchange(iter->parent);
            mem_rbt_cchange(iter->parent->parent);
            mem_rbt_cchange(tmp);
            /* Recheck the grandparent now. */
            iter = iter->parent->parent;
        }
        else if((iter->parent->parent && iter->parent->parent->left == iter->parent &&
                 iter->parent->right == iter) ||
                (iter->parent->parent && iter->parent->parent->right == iter->parent &&
                 iter->parent->left == iter)) {
            mem_rbt_cchange(iter);
            mem_rbt_cchange(iter->parent->parent);
            /* Left->Right Rotation => Double rotation. */
            mem_rbt_rotate(tree, iter);
            mem_rbt_rotate(tree, iter);
        }
        else {
            mem_rbt_cchange(iter->parent);
            mem_rbt_cchange(iter->parent->parent);
            mem_rbt_rotate(tree, iter->parent);
            /* Check parent now. */
            iter = iter->parent;
        }
    }

    /* Be sure that the root node is black. */
    tree->root->color = MEM_BLACK;
    return node;
}


void mem_rbtree_remove(mem_list_t *tree, mem_node_t *node) {
    mem_node_t *iter, *tmp_parent, *neph;
    mem_color_t color;

    SUNDRY_ASSERT(tree != NULL);
    SUNDRY_ASSERT(node != NULL);
    SUNDRY_ASSERT(node->tree == tree);
    SUNDRY_ASSERT(tree->count > 0);

    /* Remove root and return. */
    if(tree->count == 1) {
        tree->root = NULL;
        tree->count = 0;
        tree->first = NULL;
        tree->last = NULL;
        mem_rbt_reset(node);
        return;
    }

    /* If the node has both childs, we push it down to the next lower grandchild
     * which has a free right child.
     * This allows us then to remove the node because it is then an external node.
     * However, rebalancing has to be done if we removed a black child.
     */
    if(node->left && node->right) {
        /* Find the left-most grandchild which is greater than we are.
         * Then swap with this child.
         */
        iter = node->right;
        while(iter->left) iter = iter->left;

        /* We need to remember the state of the grandchild which we are goind to swap
         * to be able to correctly insert our node afterwards.
         */
        if(iter->parent == node) tmp_parent = iter;
        else tmp_parent = iter->parent;
        neph = iter->right;
        color = iter->color;

        /* Insert grandchild at our position. */
        iter->left = node->left;
        if(node->right == iter) iter->right = NULL;
        else iter->right = node->right;
        iter->parent = node->parent;
        iter->color = node->color;
        /* Update the parent's pointers. */
        if(!node->parent) tree->root = iter;
        else if(node->parent->left == node) node->parent->left = iter;
        else node->parent->right = iter;
        /* Update the child's pointers. */
        iter->left->parent = iter;
        if(iter->right) iter->right->parent = iter;
        /* Now we need to update the original parent to point to the sibling
         * or to NULL if no sibling exists.
         */
        if(tmp_parent != iter) tmp_parent->left = neph;
        else tmp_parent->right = neph;
        /* Now, we only need to rebalance the tree... */
    }
    /* Otherwise, we are by definition an external node. We can simply remove this node,
     * however, rebalancing is needed if we removed a red node.
     */
    else {
        tmp_parent = node->parent;
        if(node->left) neph = node->left;
        else neph = node->right;
        color = node->color;

        /* We need to update the parent's pointers, too. */
        if(!node->parent) tree->root = neph;
        else if(node->parent->left == node) node->parent->left = neph;
        else node->parent->right = neph;
        /* Now, we only need to rebalance the tree... */
    }

    /* If our node had have a child, we need to update it's parent pointer. */
    if(neph) neph->parent = tmp_parent;

    /* Returns the sibling of a node. */
    #define mem_sibling(parent, node) _mem_sibling((parent), (node))
    #define _mem_sibling(parent, node) ((parent->left == node)?parent->right:parent->left)

    /* Returns the "further away" and "closer" nephew of a node. */
    #define mem_nephew(parent, node) _mem_nephew((parent), (node))
    #define mem_fnephew(parent, node) _mem_fnephew((parent), (node))
    #define _mem_nephew(parent, node) ((parent->left == node)?mem_sibling(parent, node)->left:mem_sibling(parent, node)->right)
    #define _mem_fnephew(parent, node) ((parent->left == node)?mem_sibling(parent, node)->right:mem_sibling(parent, node)->left)

    /* Removing a red node never hurts, however, if we removed a black node, we
     * need to rebalance the tree.
     */
    if(color == MEM_BLACK) {
        /* Find the first red parent. */
        while(tmp_parent && (!neph || neph->color == MEM_BLACK)) {
            /* Move red siblings up and recolor them. */
            iter = mem_sibling(tmp_parent, neph);
            if(iter && iter->color == MEM_RED) {
                mem_rbt_rotate(tree, iter);
                tmp_parent->color = MEM_RED;
                tmp_parent->parent->color = MEM_BLACK;
            }

            /* If the nephews are both black now, our sibling has to be red. */
            iter = mem_nephew(tmp_parent, neph);
            if(!iter || iter->color == MEM_BLACK) {
                iter = mem_fnephew(tmp_parent, neph);
                if(!iter || iter->color == MEM_BLACK) {
                    mem_sibling(tmp_parent, neph)->color = MEM_RED;
                    /* Now balance the parent. */
                    neph = tmp_parent;
                    tmp_parent = tmp_parent->parent;
                    continue;
                }
            }

            /* One or both nephews are red, if it is the closer one, we rotate
             * it up.
             */
            iter = mem_nephew(tmp_parent, neph);
            if(iter && iter->color == MEM_RED) mem_rbt_rotate(tree, iter);

            /* Rotate our sibling up, to balance the black nodes again. */
            mem_rbt_rotate(tree, mem_sibling(tmp_parent, neph));
            tmp_parent->parent->color = tmp_parent->color;
            tmp_parent->color = MEM_BLACK;
            mem_sibling(tmp_parent->parent, tmp_parent)->color = MEM_BLACK;
            break;
        }

        /* Turn the parent red node into black. */
        if(neph && neph->color == MEM_RED) neph->color = MEM_BLACK;
    }

    /* One of \prev or \next is guaranteed to be set. */
    if(node->prev) node->prev->next = node->next;
    else tree->first = node->next;
    if(node->next) node->next->prev = node->prev;
    else tree->last = node->prev;
    --tree->count;
    mem_rbt_reset(node);
}

