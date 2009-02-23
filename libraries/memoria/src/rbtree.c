/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 23. February 2009
 */

/* Red-Black Tree implementation.
 * The RB-Tree is a binary search tree which is balanced by assigning
 * a binary color code to each node.
 */

#include "memoria/memoria.h"
#include <string.h>


/* Compares two nodes. */
static inline ons_comp_t mem_rbt_comp(mem_rbtree_t *tree, mem_rbnode_t *orig, mem_rbnode_t *comparison) {
    ons_comp_t comp;

    if(tree->match) return tree->match(orig->key, orig->klen, comparison->key, comparison->klen);
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
static mem_rbnode_t *mem_rbt_push(mem_rbtree_t *tree, mem_rbnode_t *node, bool peek) {
    mem_rbnode_t *iter;
    ons_comp_t comp;

    assert(tree != NULL);
    assert(node != NULL);

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
     * Compare \node with the node pointer to by \iter. \comp contains the
     * result and we go through this list until we find an empty node.
     */

    iter = tree->root;

    next_round:

    comp = mem_rbt_comp(tree, iter, node);
    switch(comp) {
        case ONS_SMALLER:
            if(!iter->left) {
                /* The left node is empty, we have to add the node here. */
                if(peek) return NULL;
                iter->left = node;
                break;
            }
            else {
                iter = iter->left;
                goto next_round;
            }
        case ONS_GREATER:
            if(!iter->right) {
                /* The right node is empty, add \node here. */
                if(peek) return NULL;
                iter->right = node;
                break;
            }
            else {
                iter = iter->right;
                goto next_round;
            }
        case ONS_EQUAL:
            /* The node does already exist. Return it. */
            return iter;
        default:
            assert(0);
            break;
    }

    /* Insert the node at the current iter-position. */
    ++tree->count;
    node->color = MEM_RED;
    node->tree = tree;
    node->parent = iter;

    node->left = NULL;
    node->right = NULL;
    node->next = mem_rbt_next(node);
    if(!node->next) tree->last = node;
    node->prev = mem_rbt_prev(node);
    if(!node->prev) tree->first = node;

    return node;
}

/* Rotate a node in the tree.
 * We rotate the node \node always with it's parent which causes a left
 * rotation on right nodes and vice versa.
 * Rotating the root node is simply ignored.
 */
static void mem_rbt_rotate(mem_rbtree_t *tree, mem_rbnode_t *node) {
    mem_rbnode_t *parent;

    assert(tree != NULL);
    assert(node != NULL);
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

void mem_rbt_init(mem_rbtree_t *tree, mem_rbmatch_t match) {
    assert(tree != NULL);

    tree->root = 0;
    tree->count = 0;
    tree->match = match;
    tree->first = NULL;
    tree->last = NULL;
}

void mem_rbt_clear(mem_rbtree_t *tree, void (*del_func)(void*)) {
    assert(tree != NULL);

    if(del_func) while(tree->count) del_func(mem_rbt_del(tree, tree->root));
    else while(tree->count) mem_rbt_del(tree, tree->root);
}

/* Returns the uncle of a node.
 * Returns NULL if no uncle is present.
 */
static inline mem_rbnode_t *mem_rbt_uncle(mem_rbnode_t *node) {
    if(!node->parent || !node->parent->parent) return NULL;
    if(node->parent->parent->left == node->parent) return node->parent->parent->right;
    else return node->parent->parent->left;
}

/* Changes the color of a node. */
#define mem_rbt_cchange(node) ((node)->color = ((node)->color == MEM_RED) ? MEM_BLACK : MEM_RED)

bool mem_rbt_add(mem_rbtree_t *tree, const char *key, size_t klen, void *data, mem_rbnode_t **result) {
    mem_rbnode_t *node, *iter, *tmp;

    assert(tree != NULL);
    assert(key != NULL);
    assert(klen != 0);
    assert(data != NULL);
    assert(result != NULL);

    node = mem_malloc(sizeof(mem_rbnode_t) + klen + 1);
    node->key = sizeof(mem_rbnode_t) + (void*)node;
    node->klen = klen;
    node->data = data;
    memcpy(node->key, key, klen);
    /* For convenience reasons we add a terminating zero character at the end.
     * This character is not included in \klen.
     */
    node->key[klen] = 0;

    /* This key does already exist. We return the node. */
    if((tmp = mem_rbt_push(tree, node, false)) != node) {
        mem_free(node);
        *result = tmp;
        return false;
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

    *result = node;
    return true;
}

void *mem_rbt_del(mem_rbtree_t *tree, mem_rbnode_t *node) {
    void *ret;
    mem_rbnode_t *iter, *tmp_parent, *neph;
    mem_rbcolor_t color;

    assert(tree != NULL);
    assert(node != NULL);

    ret = node->data;

    /* Remove root and return. */
    if(tree->count == 1) {
        tree->root = NULL;
        tree->count = 0;
        tree->first = NULL;
        tree->last = NULL;
        mem_free(node);
        return ret;
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

    mem_free(node);
    --tree->count;
    return ret;
}

mem_rbnode_t *mem_rbt_find(mem_rbtree_t *tree, const char *key, size_t klen) {
    mem_rbnode_t *node, find;

    assert(tree != NULL);
    assert(key != NULL);
    assert(klen != 0);

    find.key = (char*)key;
    find.klen = klen;

    node = mem_rbt_push(tree, &find, true);
    return node;
}

mem_rbnode_t *mem_rbt_first(mem_rbtree_t *tree) {
    mem_rbnode_t *iter;

    assert(tree != NULL);
    if(!tree->root) return NULL;

    iter = tree->root;
    while(iter->left) iter = iter->left;
    return iter;
}

mem_rbnode_t *mem_rbt_last(mem_rbtree_t *tree) {
    mem_rbnode_t *iter;

    assert(tree != NULL);
    if(!tree->root) return NULL;

    iter = tree->root;
    while(iter->right) iter = iter->right;
    return iter;
}

mem_rbnode_t *mem_rbt_next(mem_rbnode_t *node) {
    assert(node != NULL);

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

mem_rbnode_t *mem_rbt_prev(mem_rbnode_t *node) {
    assert(node != NULL);

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

