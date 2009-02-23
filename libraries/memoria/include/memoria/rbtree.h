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


struct mem_rbtree_t;
struct mem_rbnode_t;

typedef ons_comp_t (*mem_rbmatch_t)(const void *orig, const void *comparison);

/* The color of a node. */
typedef enum mem_rbcolor_t {
    MEM_RED,
    MEM_BLACK
} mem_rbcolor_t;

typedef struct mem_rbtree_t {
    struct mem_rbnode_t *root;
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
} mem_rbnode_t;

extern void mem_rbt_init(mem_rbtree_t *tree, mem_rbmatch_t match);
extern bool mem_rbt_add(mem_rbtree_t *tree, const char *key, size_t klen, void *data, mem_rbnode_t **result);
extern void *mem_rbt_del(mem_rbtree_t *tree, mem_rbnode_t *node);
extern mem_rbnode_t *mem_rbt_find(mem_rbtree_t *tree, const char *key, size_t klen);




#if 0
    long rbtree_comp(rb_tree_t *tree, db_key_t *key1, db_key_t *key2);
    struct _rb_tree_s {
      unsigned long	rt_magic;	/**< Magic number. */
      unsigned long	rt_flags;	/**< Flags associated with the table. */
      unsigned long	rt_count;	/**< Number of nodes in the tree. */
      rb_node_t    *rt_root;	/**< Pointer to the root node of the tree. */
      rb_comp_t	rt_comp;	/**< Function for comparing tree keys. */
      void	       *rt_extra;	/**< Extra data associated with the tree. */
    };
    #define RB_TREE_MAGIC 0xd52695be
    #define RBT_FLAG_FREEZE 0x80000000 /* tree frozen */
    #define RB_TREE_INIT(comp, extra)   { RB_TREE_MAGIC, 0, 0, 0, (comp), (extra) }
    #define rt_verify(tree)		((tree) && \
                     (tree)->rt_magic == RB_TREE_MAGIC)
    #define rt_frozen(tree)		((tree)->rt_flags & RBT_FLAG_FREEZE)
    #define rt_count(tree)		((tree)->rt_count)
    #define rt_root(tree)		((tree)->rt_root)
    #define rt_comp(tree)		((tree)->rt_comp)
    #define rt_extra(tree)		((tree)->rt_extra)
    #define RBT_ORDER_PRE	1
    #define RBT_ORDER_IN	2
    #define RBT_ORDER_POST	3
    #define RBT_ORDER_MASK	0x00000003
    #define rt_prev(tree, node_io, flags) \
        (rt_next((tree), (node_io), (flags) ^ DB_FLAG_REVERSE))
    unsigned long rt_init(rb_tree_t *tree, rb_comp_t comp, void *extra);
    unsigned long rt_add(rb_tree_t *tree, rb_node_t *node, db_key_t *key);
    unsigned long rt_move(rb_tree_t *tree, rb_node_t *node, db_key_t *key);
    unsigned long rt_remove(rb_tree_t *tree, rb_node_t *node);
    unsigned long rt_find(rb_tree_t *tree, rb_node_t **node_p, db_key_t *key);
    unsigned long rt_next(rb_tree_t *tree, rb_node_t **node_io,
                  unsigned long flags);
    unsigned long rt_iter(rb_tree_t *tree, rb_node_t *start,
                  rb_iter_t iter_func, void *extra, unsigned long flags);
    unsigned long rt_flush(rb_tree_t *tree, rb_iter_t flush_func, void *extra);

    struct _rb_node_s {
      unsigned long	rn_magic;	/**< Magic number. */
      rb_color_t	rn_color;	/**< Node's color. */
      rb_tree_t    *rn_tree;	/**< Tree node is in. */
      rb_node_t    *rn_parent;	/**< Parent of this node. */
      rb_node_t    *rn_left;	/**< Left child of this node. */
      rb_node_t    *rn_right;	/**< Right child of this node. */
      db_key_t	rn_key;		/**< Node's key. */
      void	       *rn_value;	/**< Node's value. */
    };
    #define RB_NODE_MAGIC 0x3dea4d01
    #define RB_NODE_INIT(value) \
        { RB_NODE_MAGIC, RB_COLOR_NONE, 0, 0, 0, 0, DB_KEY_INIT(0, 0), (value)}
    #define rn_verify(node)		((node) && \
                     (node)->rn_magic == RB_NODE_MAGIC)
    #define rn_color(node)		((node)->rn_color)
    #define rn_tree(node)		((node)->rn_tree)
    #define rn_parent(node)		((node)->rn_parent)
    #define rn_left(node)		((node)->rn_left)
    #define rn_right(node)		((node)->rn_right)
    #define rn_key(node)		(&((node)->rn_key))
    #define rn_value(node)		((node)->rn_value)
    #define rn_isblack(node)	(!(node) || (node)->rn_color == RB_COLOR_BLACK)
    #define rn_isred(node)		((node) && (node)->rn_color == RB_COLOR_RED)
    #define rn_isleft(node)		((node)->rn_parent && \
                     (node)->rn_parent->rn_left == (node))
    #define rn_isright(node)	((node)->rn_parent && \
                     (node)->rn_parent->rn_right == (node))

    unsigned long rn_init(rb_node_t *node, void *value);

    rb_node_t *_rb_locate(rb_tree_t *tree, rb_node_t *node, db_key_t *key);
    void _rb_rotate(rb_tree_t *tree, rb_node_t *child);
#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_rbtree_h */

