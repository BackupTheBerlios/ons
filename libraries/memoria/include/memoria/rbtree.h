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

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_rbtree_h
#define ONS_INCLUDED_memoria_rbtree_h
ONS_EXTERN_C_BEGIN


#include <memoria/string.h>

struct mem_rbtree_t;
struct mem_rbnode_t;

typedef ons_comp_t (*mem_rbmatch_t)(const void *orig_key, size_t orig_len, const void *comp_key, size_t comp_len);

/* The color of a node. */
typedef enum mem_rbcolor_t {
    MEM_RED,
    MEM_BLACK
} mem_rbcolor_t;

typedef struct mem_rbtree_t {
    struct mem_rbnode_t *root;
    struct mem_rbnode_t *first;
    struct mem_rbnode_t *last;
    size_t count;
    mem_rbmatch_t match;
} mem_rbtree_t;

typedef struct mem_rbnode_t {
    char *key;
    size_t klen;
    void *data;
    mem_rbcolor_t color;

    mem_rbtree_t *tree;
    struct mem_rbnode_t *parent;

    struct mem_rbnode_t *left;
    struct mem_rbnode_t *right;

    struct mem_rbnode_t *next;
    struct mem_rbnode_t *prev;
} mem_rbnode_t;

#define MEM_STR(str) (str), strlen(str)
#define MEM_LIT(str) (str), sizeof(str)
#define MEM_STRN(str, len) (str), strnlen((str), (len))
extern void mem_rbt_init(mem_rbtree_t *tree, mem_rbmatch_t match);
extern void mem_rbt_clear(mem_rbtree_t *tree, void (*del_func)(void*));
extern bool mem_rbt_add(mem_rbtree_t *tree, const char *key, size_t klen, void *data, mem_rbnode_t **result);
extern void *mem_rbt_del(mem_rbtree_t *tree, mem_rbnode_t *node);
extern mem_rbnode_t *mem_rbt_find(mem_rbtree_t *tree, const char *key, size_t klen);

extern mem_rbnode_t *mem_rbt_first(mem_rbtree_t *tree);
extern mem_rbnode_t *mem_rbt_last(mem_rbtree_t *tree);
extern mem_rbnode_t *mem_rbt_next(mem_rbnode_t *node);
extern mem_rbnode_t *mem_rbt_prev(mem_rbnode_t *node);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_rbtree_h */

