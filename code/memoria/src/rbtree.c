/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. June 2009
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
    while(iter->be.rbtree.left) iter = iter->be.rbtree.left;
    return iter;
}
mem_node_t *mem_rbt_last(mem_list_t *tree) {
    mem_node_t *iter;

    SUNDRY_ASSERT(tree != NULL);
    if(!tree->root) return NULL;

    iter = tree->root;
    while(iter->be.rbtree.right) iter = iter->be.rbtree.right;
    return iter;
}
mem_node_t *mem_rbt_next(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);

    if(node->be.rbtree.right) {
        node = node->be.rbtree.right;
        while(node->be.rbtree.left) node = node->be.rbtree.left;
        return node;
    }
    else if(!node->be.rbtree.parent) return NULL;
    else if(node->be.rbtree.parent->be.rbtree.left == node) return node->be.rbtree.parent;
    else {
        node = node->be.rbtree.parent;
        while(node->be.rbtree.parent && node->be.rbtree.parent->be.rbtree.right == node) node = node->be.rbtree.parent;
        if(!node->be.rbtree.parent) return NULL;
        else return node->be.rbtree.parent;
    }
}
mem_node_t *mem_rbt_prev(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);

    if(node->be.rbtree.left) {
        node = node->be.rbtree.left;
        while(node->be.rbtree.right) node = node->be.rbtree.right;
        return node;
    }
    else if(!node->be.rbtree.parent) return NULL;
    else if(node->be.rbtree.parent->be.rbtree.right == node) return node->be.rbtree.parent;
    else {
        node = node->be.rbtree.parent;
        while(node->be.rbtree.parent && node->be.rbtree.parent->be.rbtree.left == node) node = node->be.rbtree.parent;
        if(!node->be.rbtree.parent) return NULL;
        else return node->be.rbtree.parent;
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

        node->be.rbtree.color = MEM_BLACK;
        node->be.rbtree.parent = NULL;
        node->be.rbtree.tree = tree;
        node->be.rbtree.left = NULL;
        node->be.rbtree.right = NULL;
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
        if(!iter->be.rbtree.left) {
            /* The left node is empty, we have to add the node here. */
            if(peek) return NULL;
            iter->be.rbtree.left = node;
        }
        else {
            iter = iter->be.rbtree.left;
            goto next_round;
        }
    }
    else if(comp > 0) {
        if(!iter->be.rbtree.right) {
            /* The right node is empty, add \node here. */
            if(peek) return NULL;
            iter->be.rbtree.right = node;
        }
        else {
            iter = iter->be.rbtree.right;
            goto next_round;
        }
    }
    else {
        /* The node does already exist. Return it. */
        return iter;
    }

    /* Insert the node at the current iter-position. */
    ++tree->count;
    node->be.rbtree.color = MEM_RED;
    node->be.rbtree.tree = tree;
    node->be.rbtree.parent = iter;

    node->be.rbtree.left = NULL;
    node->be.rbtree.right = NULL;
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
    if(!node->be.rbtree.parent) return;

    parent = node->be.rbtree.parent;

    /* Repoint the parent of the parent to the node, except if the grandparent is
     * the root node, then we simply let the root node point on the node.
     */
    if(!parent->be.rbtree.parent) tree->root = node;
    /* Check for right rotation. */
    else if(parent->be.rbtree.parent->be.rbtree.left == parent) parent->be.rbtree.parent->be.rbtree.left = node;
    /* Must be a left rotation. */
    else parent->be.rbtree.parent->be.rbtree.right = node;

    /* Update the parent pointers. */
    node->be.rbtree.parent = parent->be.rbtree.parent;
    parent->be.rbtree.parent = node;

    /* On right rotation, move the right child of \node to the left childs of the old parent.
     * On left rotation, vice versa.
     */
    if(parent->be.rbtree.right == node) {
        /* left rotation */
        if(node->be.rbtree.left) node->be.rbtree.left->be.rbtree.parent = parent;
        parent->be.rbtree.right = node->be.rbtree.left;
        node->be.rbtree.left = parent;
    }
    else {
        /* right rotation */
        if(node->be.rbtree.right) node->be.rbtree.right->be.rbtree.parent = parent;
        parent->be.rbtree.left = node->be.rbtree.right;
        node->be.rbtree.right = parent;
    }
}


/* Returns the uncle of a node.
 * Returns NULL if no uncle is present.
 */
static mem_node_t *mem_rbt_uncle(mem_node_t *node) {
    if(!node->be.rbtree.parent || !node->be.rbtree.parent->be.rbtree.parent) return NULL;
    if(node->be.rbtree.parent->be.rbtree.parent->be.rbtree.left == node->be.rbtree.parent)
        return node->be.rbtree.parent->be.rbtree.parent->be.rbtree.right;
    else return node->be.rbtree.parent->be.rbtree.parent->be.rbtree.left;
}

/* Changes the color of a node. */
#define mem_rbt_cchange(node) ((node)->be.rbtree.color = ((node)->be.rbtree.color == MEM_RED) ? MEM_BLACK : MEM_RED)

/* Resets a node to NULL except key/len/data. */
static void mem_rbt_reset(mem_node_t *node) {
    SUNDRY_ASSERT(node != NULL);
    node->next = NULL;
    node->prev = NULL;
    node->be.rbtree.color = MEM_RED;
    node->be.rbtree.tree = NULL;
    node->be.rbtree.parent = NULL;
    node->be.rbtree.left = NULL;
    node->be.rbtree.right = NULL;
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
    SUNDRY_ASSERT(node->be.rbtree.parent == NULL && node->be.rbtree.tree == NULL);

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
    while(iter && iter->be.rbtree.color != MEM_BLACK && iter->be.rbtree.parent && iter->be.rbtree.parent->be.rbtree.color != MEM_BLACK) {
        tmp = mem_rbt_uncle(iter);
        if(tmp && tmp->be.rbtree.color == MEM_RED) {
            mem_rbt_cchange(iter->be.rbtree.parent);
            mem_rbt_cchange(iter->be.rbtree.parent->be.rbtree.parent);
            mem_rbt_cchange(tmp);
            /* Recheck the grandparent now. */
            iter = iter->be.rbtree.parent->be.rbtree.parent;
        }
        else if((iter->be.rbtree.parent->be.rbtree.parent && iter->be.rbtree.parent->be.rbtree.parent->be.rbtree.left == iter->be.rbtree.parent &&
                 iter->be.rbtree.parent->be.rbtree.right == iter) ||
                (iter->be.rbtree.parent->be.rbtree.parent && iter->be.rbtree.parent->be.rbtree.parent->be.rbtree.right == iter->be.rbtree.parent &&
                 iter->be.rbtree.parent->be.rbtree.left == iter)) {
            mem_rbt_cchange(iter);
            mem_rbt_cchange(iter->be.rbtree.parent->be.rbtree.parent);
            /* Left->be.rbtree.right Rotation => Double rotation. */
            mem_rbt_rotate(tree, iter);
            mem_rbt_rotate(tree, iter);
        }
        else {
            mem_rbt_cchange(iter->be.rbtree.parent);
            mem_rbt_cchange(iter->be.rbtree.parent->be.rbtree.parent);
            mem_rbt_rotate(tree, iter->be.rbtree.parent);
            /* Check parent now. */
            iter = iter->be.rbtree.parent;
        }
    }

    /* Be sure that the root node is black. */
    tree->root->be.rbtree.color = MEM_BLACK;
    return node;
}


void mem_rbtree_remove(mem_list_t *tree, mem_node_t *node) {
    mem_node_t *iter, *tmp_parent, *neph;
    mem_color_t color;

    SUNDRY_ASSERT(tree != NULL);
    SUNDRY_ASSERT(node != NULL);
    SUNDRY_ASSERT(node->be.rbtree.tree == tree);
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
    if(node->be.rbtree.left && node->be.rbtree.right) {
        /* Find the left-most grandchild which is greater than we are.
         * Then swap with this child.
         */
        iter = node->be.rbtree.right;
        while(iter->be.rbtree.left) iter = iter->be.rbtree.left;

        /* We need to remember the state of the grandchild which we are goind to swap
         * to be able to correctly insert our node afterwards.
         */
        if(iter->be.rbtree.parent == node) tmp_parent = iter;
        else tmp_parent = iter->be.rbtree.parent;
        neph = iter->be.rbtree.right;
        color = iter->be.rbtree.color;

        /* Insert grandchild at our position. */
        iter->be.rbtree.left = node->be.rbtree.left;
        if(node->be.rbtree.right == iter) iter->be.rbtree.right = NULL;
        else iter->be.rbtree.right = node->be.rbtree.right;
        iter->be.rbtree.parent = node->be.rbtree.parent;
        iter->be.rbtree.color = node->be.rbtree.color;
        /* Update the parent's pointers. */
        if(!node->be.rbtree.parent) tree->root = iter;
        else if(node->be.rbtree.parent->be.rbtree.left == node) node->be.rbtree.parent->be.rbtree.left = iter;
        else node->be.rbtree.parent->be.rbtree.right = iter;
        /* Update the child's pointers. */
        iter->be.rbtree.left->be.rbtree.parent = iter;
        if(iter->be.rbtree.right) iter->be.rbtree.right->be.rbtree.parent = iter;
        /* Now we need to update the original parent to point to the sibling
         * or to NULL if no sibling exists.
         */
        if(tmp_parent != iter) tmp_parent->be.rbtree.left = neph;
        else tmp_parent->be.rbtree.right = neph;
        /* Now, we only need to rebalance the tree... */
    }
    /* Otherwise, we are by definition an external node. We can simply remove this node,
     * however, rebalancing is needed if we removed a red node.
     */
    else {
        tmp_parent = node->be.rbtree.parent;
        if(node->be.rbtree.left) neph = node->be.rbtree.left;
        else neph = node->be.rbtree.right;
        color = node->be.rbtree.color;

        /* We need to update the parent's pointers, too. */
        if(!node->be.rbtree.parent) tree->root = neph;
        else if(node->be.rbtree.parent->be.rbtree.left == node) node->be.rbtree.parent->be.rbtree.left = neph;
        else node->be.rbtree.parent->be.rbtree.right = neph;
        /* Now, we only need to rebalance the tree... */
    }

    /* If our node had have a child, we need to update it's parent pointer. */
    if(neph) neph->be.rbtree.parent = tmp_parent;

    /* Returns the sibling of a node. */
    #define mem_sibling(parent, node) _mem_sibling((parent), (node))
    #define _mem_sibling(parent, node) ((parent->be.rbtree.left == node)?parent->be.rbtree.right:parent->be.rbtree.left)

    /* Returns the "further away" and "closer" nephew of a node. */
    #define mem_nephew(parent, node) _mem_nephew((parent), (node))
    #define mem_fnephew(parent, node) _mem_fnephew((parent), (node))
    #define _mem_nephew(parent, node) ((parent->be.rbtree.left == node)?mem_sibling(parent, node)->be.rbtree.left:mem_sibling(parent, node)->be.rbtree.right)
    #define _mem_fnephew(parent, node) ((parent->be.rbtree.left == node)?mem_sibling(parent, node)->be.rbtree.right:mem_sibling(parent, node)->be.rbtree.left)

    /* Removing a red node never hurts, however, if we removed a black node, we
     * need to rebalance the tree.
     */
    if(color == MEM_BLACK) {
        /* Find the first red parent. */
        while(tmp_parent && (!neph || neph->be.rbtree.color == MEM_BLACK)) {
            /* Move red siblings up and recolor them. */
            iter = mem_sibling(tmp_parent, neph);
            if(iter && iter->be.rbtree.color == MEM_RED) {
                mem_rbt_rotate(tree, iter);
                tmp_parent->be.rbtree.color = MEM_RED;
                tmp_parent->be.rbtree.parent->be.rbtree.color = MEM_BLACK;
            }

            /* If the nephews are both black now, our sibling has to be red. */
            iter = mem_nephew(tmp_parent, neph);
            if(!iter || iter->be.rbtree.color == MEM_BLACK) {
                iter = mem_fnephew(tmp_parent, neph);
                if(!iter || iter->be.rbtree.color == MEM_BLACK) {
                    mem_sibling(tmp_parent, neph)->be.rbtree.color = MEM_RED;
                    /* Now balance the parent. */
                    neph = tmp_parent;
                    tmp_parent = tmp_parent->be.rbtree.parent;
                    continue;
                }
            }

            /* One or both nephews are red, if it is the closer one, we rotate
             * it up.
             */
            iter = mem_nephew(tmp_parent, neph);
            if(iter && iter->be.rbtree.color == MEM_RED) mem_rbt_rotate(tree, iter);

            /* Rotate our sibling up, to balance the black nodes again. */
            mem_rbt_rotate(tree, mem_sibling(tmp_parent, neph));
            tmp_parent->be.rbtree.parent->be.rbtree.color = tmp_parent->be.rbtree.color;
            tmp_parent->be.rbtree.color = MEM_BLACK;
            mem_sibling(tmp_parent->be.rbtree.parent, tmp_parent)->be.rbtree.color = MEM_BLACK;
            break;
        }

        /* Turn the parent red node into black. */
        if(neph && neph->be.rbtree.color == MEM_RED) neph->be.rbtree.color = MEM_BLACK;
    }

    /* One of \prev or \next is guaranteed to be set. */
    if(node->prev) node->prev->next = node->next;
    else tree->first = node->next;
    if(node->next) node->next->prev = node->prev;
    else tree->last = node->prev;
    --tree->count;
    mem_rbt_reset(node);
}

