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

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_splay_tree_h
#define ONS_INCLUDED_memoria_splay_tree_h
ONS_EXTERN_C_BEGIN


#include <memoria/string.h>
#include <memoria/rbtree.h>

struct mem_stree_t;
struct mem_snode_t;

typedef ons_comp_t (*mem_smatch_t)(const struct mem_snode_t *orig, const struct mem_snode_t *comparison);

typedef struct mem_stree_t {
    struct mem_snode_t *root;
    size_t count;
    mem_smatch_t match;
} mem_stree_t;

typedef struct mem_snode_t {
    char *key;
    size_t klen;
    void *data;

    struct mem_snode_t *left;
    struct mem_snode_t *right;
} mem_snode_t;

extern void mem_stree_init(mem_stree_t *tree, mem_smatch_t match);
extern bool mem_stree_add(mem_stree_t *tree, const char *key, size_t klen, void *data, mem_snode_t **result);
extern void *mem_stree_del(mem_stree_t *tree, mem_snode_t *node);
extern mem_snode_t *mem_stree_find(mem_stree_t *tree, const char *key, size_t klen);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_splay_tree_h */

