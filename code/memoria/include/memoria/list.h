/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. June 2009
 */

/* List interface
 * This interface implements a generic API with several backends which
 * allow to access the elements as a double linked list, search for
 * elements and inserting/deleting elements.
 * The backends include a simple double linked list, hash tables, binary
 * search trees, splay trees, red-black trees and others.
 */


#include <sundry/sundry.h>

#ifndef MEMORIA_INCLUDED_memoria_list_h
#define MEMORIA_INCLUDED_memoria_list_h
SUNDRY_EXTERN_C_BEGIN


#include <stdint.h>
#include <memoria/memoria.h>
#include <memoria/alloc.h>

/* The color of an RBTree node. */
typedef enum mem_color_t {
    MEM_RED,
    MEM_BLACK
} mem_color_t;


/* Backend structure. */
typedef union mem_node_be_t {
    struct mem_node_be_rb_t {
        struct mem_node_t *left;
        struct mem_node_t *right;
        mem_color_t color;
        struct mem_list_t *tree;
        struct mem_node_t *parent;
    } rbtree;
    struct mem_node_be_splay_t {
        struct mem_node_t *left;
        struct mem_node_t *right;
    } splay;
} mem_node_be_t;


/* Single node which is stored in a linked list. */
typedef struct mem_node_t {
    /* node properties */
    void *key;
    size_t len;
    void *value;

    /* linked list */
    struct mem_node_t *next;
    struct mem_node_t *prev;

    /* backend; this must be the last member in the structure! */
    mem_node_be_t be;
} mem_node_t;


/* A list holding an unlimited amount of nodes.
 * The \match function gets two mem_node_t structs as parameters and must
 * not use any other members than \key and \len.
 */
typedef struct mem_list_t {
    /* properties */
    unsigned int type;
    mem_match_t match;
    void (*delfunc)(mem_node_t*);
    size_t count;

    /* list */
    mem_node_t *first;
    mem_node_t *last;

    /* backend */
    mem_node_t *root;
} mem_list_t;


/* Functions which operate on a single node. */
extern void mem_node_init(mem_node_t *node, unsigned int type);
extern void mem_node_free(mem_node_t *node);
#define mem_node_key(node) (node)->key
#define mem_node_len(node) (node)->len
#define mem_node_value(node) (node)->value
#define mem_node_next(node) (node)->next
#define mem_node_prev(node) (node)->prev


/* Functions to manage an whole list. */
extern void mem_list_init(mem_list_t *list, unsigned int type);
extern void mem_list_clear(mem_list_t *list);
extern void mem_list_free(mem_list_t *list);


/* These functions handle node<->list relationships. */
extern mem_node_t *mem_list_find(mem_list_t *list, void *key, size_t len);
extern mem_node_t *mem_list_insert(mem_list_t *list, mem_node_t *node);
extern void mem_list_remove(mem_list_t *list, mem_node_t *node);


/* Backends
 * There are several backends behind a list interface. They differ in their algorithms
 * and hence they differ in runtime behaviour. Some are very fast in searching but have
 * bigger node sizes, others have extremely small node sizes but are somewhat slower
 * in searching. See each backend for detailed description.
 *
 * New backends:
 * - Add a new part to the union of the mem_backend_t structure. This must contain all
 *   information you need in your backend when handling a node.
 * - Add a new part to the "backend" union in the mem_list_t structure. It must contains
 *   information necessary to handle a list with your backend.
 * - Fill a "mem_binfo_t" structure with the information for the backend and add it into
 *   the mem_blist assigned with a new key.
 */


/* Information about a single backend. */
typedef struct {
    size_t size;                                                    /* Size of the mem_backend_t structure. */
    void (*clear)(mem_list_t *list);                                /* Function that clears a list. */
    mem_node_t *(*find)(mem_list_t *list, void *key, size_t len);   /* Function that finds a node in a list. */
    mem_node_t *(*insert)(mem_list_t *list, mem_node_t *node);      /* Function that inserts a node into a list. */
    void (*remove)(mem_list_t *list, mem_node_t *node);             /* Function that removes a node from a list. */
} mem_binfo_t;
#define MEM_BSIZE(type) (mem_blist[type]->size)


/* Red-Black Tree backend. */
extern void mem_rbtree_clear(mem_list_t *list);
extern mem_node_t *mem_rbtree_find(mem_list_t *list, void *key, size_t len);
extern mem_node_t *mem_rbtree_insert(mem_list_t *list, mem_node_t *node);
extern void mem_rbtree_remove(mem_list_t *list, mem_node_t *node);
extern mem_binfo_t mem_rbtree;


/* Splay Tree backend. */
extern void mem_splay_clear(mem_list_t *list);
extern mem_node_t *mem_splay_find(mem_list_t *list, void *key, size_t len);
extern mem_node_t *mem_splay_insert(mem_list_t *list, mem_node_t *node);
extern void mem_splay_remove(mem_list_t *list, mem_node_t *node);
extern mem_binfo_t mem_splay;


/* Hash table backend. */
extern void mem_table_clear(mem_list_t *list);
extern mem_node_t *mem_table_find(mem_list_t *list, void *key, size_t len);
extern mem_node_t *mem_table_insert(mem_list_t *list, mem_node_t *node);
extern void mem_table_remove(mem_list_t *list, mem_node_t *node);
extern mem_binfo_t mem_table;


/* Array of all backends. */
enum {
    MEM_RBTREE,
    MEM_SPLAY,
    MEM_BACKEND_LAST
};
extern mem_binfo_t *mem_blist[MEM_BACKEND_LAST];


/* Returns true if the type is valid, otherwise false. */
#define mem_valid_type(type) ((type) < MEM_BACKEND_LAST)


/* Double Linked List Interface
 * These functions help managing all kinds of linked lists. There have been many
 * approaches to realize linked-lists with macros. However, several problems occured
 * which made it impossible to realize a *REAL* linked list interface with macros.
 * I don't explain this now, but when you think you have such an interface, try
 * calling your macros with parameters like "node->next->next" or "node->prev->next"
 * or something else and you may notice that this *REALLY* does not work.
 *
 * This approach now implements the core functions as *REAL* C functions which are, of course,
 * inlined and static. This allows to avoid the problem with macros that they do not
 * save a copy of the arguments. However, plain C functions do not allow to use with
 * arbitrary types or (like ours) have weird parameters which are really bad to use.
 *
 * So we provided macros that make the usage of the core much more convenient. The interface
 * still works with arbitrary datatypes. However, we provide 4 ways which try to generalize
 * the datatypes:
 *   The datatype you want to store in the linked list must be a structure with two members
 *   which point to the previous and next element. They are generally called "next" and "prev"
 *   but you can also use other names.
 *
 *   The base structure which holds the linked list normally has a pointer called "first" which
 *   points to the first node in the list and a pointer called "last" which points to the
 *   last node in the list. However, you can also use other names and you can even use a linked
 *   list without such a base structure.
 *
 *   The base structure often has a "count" member which counts the amount of members in the list
 *   Such a variable is also optionally supported.
 *
 * The linked list is *NOT* circular. That means, the "prev" pointer of the first node is NULL
 * and the "next" pointer of the last node is NULL.
 * If you use the "first" and "last" pointers, then both are NULL if the list is empty, otherwise
 * they always point to the related nodes.
 *
 * This API does not care for allocating/freeing nodes. That is, adding a node just links an
 * existing node into the list. Removing a node just unlinks a node from a list. Thus, you can
 * link a node into the list, remove it, link it again and so on without reallocating space.
 * This also means that you may have nodes which are not linked into a list or which are linked
 * in more than one list.
 * It is important to know that the "next" and "prev" members of a node MUST be NULL when the
 * node is not linked into a list.
 * Linking into more than one list can be realized by adding additional "next" and "prev" members
 * which may be called "next2" or "special_next" or something else.
 *
 * The API is built up that you can do four general tasks on a linked list:
 *   - (ADD) Adding new nodes into the list.
 *   - (DEL) Removing nodes from the list.
 *   - (FIND) Finding in the list and iterating through the list.
 *   - (SORT) Sorting the list.
 * Each task has several functions which fullfil your needs.
 *
 * ADD: Adding new nodes into the list.
 *   Four ways to insert a node are provided.
 *    - Pushing a node to the end of the list.
 *    - Adding a node to the beginning of the list.
 *    - Append a node after an existing node.
 *    - Prepend a node before an existing node.
 * Moving a node should be avoided and is not supported diretcly. Please remove the node from the
 * list and add it at another position.
 *
 * DEL: Removing a node from a list.
 *   One function is provided.
 *    - You can specify a node directly which should be removed.
 * TODO: clean: You can remove all nodes from the list at once.
 *
 * FIND: Finding a node in a list or iterating though all nodes of a list.
 *   TODO: Currently no special functions are supported. Please iterate through
 *   the list with the "next" and "prev" pointers.
 *
 * SORT: Sorting a list.
 *   TODO: Sorting a list is currently not supported.
 *
 * API:
 * Depending on your structure we have several APIs which reduce the number of parameters and
 * ease the usage. The base is an API which can be used with ALL datatypes. The following
 * APIs are available:
 *    - Generic: Works with all datatypes but is rather complex.
 *    - Named node: Expects the node to have the "next" and "prev" pointers exactly named "next"/"prev".
 *    - With base: Expects a base structure which holds the node. It must have members pointing to the
 *                 first and last members.
 *    - With named base: Same as "with base" but the members must be called "first" and "last".
 *    ... and any combination of them.
 * The function names are built up the following way:
 *    mem_llist_<api>_<function>().
 * Whereas <function> is one of:
 *    - prepend, append, push, thrust, remove
 * as they are described above.
 * <api> is one of:
 *    -   g: generic API.
 *    -   b: generic API with base.
 *    -   n: named generic API.
 *    -  nb: generic API with named base.
 *    -  bn: named generic API with base.
 *    - nbn: named generic API with named base.
 */

/* GENERIC API */
#define mem_llist_g_prepend(next, prev, first, ref, node) mem_llist_prepend((first), \
                                                                            (ref), \
                                                                            &(ref)->prev, \
                                                                            ((ref)->prev)?(&(ref)->prev->next):NULL, \
                                                                            (node), \
                                                                            &(node)->next, \
                                                                            &(node)->prev)
#define mem_llist_g_append(next, prev, last, ref, node) mem_llist_append((last), \
                                                                         (ref), \
                                                                         &(ref)->next, \
                                                                         ((ref)->next)?(&(ref)->next->prev):NULL, \
                                                                         (node), \
                                                                         &(node)->next, \
                                                                         &(node)->prev)
#define mem_llist_g_push(next, prev, last, node) mem_llist_push((last), \
                                                                (last)?(&(last)->next):NULL, \
                                                                (node), \
                                                                &(node)->next, \
                                                                &(node)->prev)
#define mem_llist_g_thrust(next, prev, first, node) mem_llist_thrust((first), \
                                                                     (first)?(&(first)->prev):NULL, \
                                                                     (node), \
                                                                     &(node)->next, \
                                                                     &(node)->prev)
#define mem_llist_g_remove(next, prev, node) mem_llist_remove((node), &(node)->next, &(node)->prev, \
                                                              ((node)->prev)?&(node)->prev->next:NULL, \
                                                              ((node)->next)?&(node)->next->prev:NULL)

/* GENERIC API with BASE structure */
#define mem_llist_b_prepend(first, last, next, prev, base, ref, node) \
    (((base)->first) \
        ?((base)->first = mem_llist_g_prepend(next, prev, (base)->first, (ref), (node))) \
        :((base)->first = (base)->last = mem_llist_g_prepend(next, prev, (base)->first, (ref), (node))))
#define mem_llist_b_append(first, last, next, prev, base, ref, node) \
    (((base)->first) \
        ?((base)->first = mem_llist_g_append(next, prev, (base)->last, (ref), (node))) \
        :((base)->first = (base)->last = mem_llist_g_append(next, prev, (base)->last, (ref), (node))))
#define mem_llist_b_push(first, last, next, prev, base, node) \
    (((base)->last) \
        ?((base)->last = mem_llist_g_push(next, prev, (base)->last, (node))) \
        :((base)->first = (base)->last = mem_llist_g_push(next, prev, (base)->last, (node))))
#define mem_llist_b_thrust(first, last, next, prev, base, node) \
    (((base)->first) \
        ?((base)->first = mem_llist_g_thrust(next, prev, (base)->first, (node))) \
        :((base)->first = (base)->last = mem_llist_g_thrust(next, prev, (base)->first, (node))))
#define mem_llist_b_remove(first, last, next, prev, base, node) \
    ((((base)->first == (node)) \
        ?(((base)->last == (node)) \
            ?((base)->first = (base)->last = NULL) \
            :((base)->first = (node)->next)) \
        :(((base)->last == (node)) \
            ?((base)->last = (node)->prev) \
            :0)), \
    mem_llist_g_remove(next, prev, node))

/* NAMED GENERIC API */
#define mem_llist_n_prepend(first, ref, node) mem_llist_g_prepend(next, prev, (first), (ref), (node))
#define mem_llist_n_append(last, ref, node) mem_llist_g_append(next, prev, (last), (ref), (node))
#define mem_llist_n_push(last, node) mem_llist_g_push(next, prev, (last), (node))
#define mem_llist_n_thrust(first, node) mem_llist_g_thrust(next, prev, (first), (node))
#define mem_llist_n_remove(node) mem_llist_g_remove(next, prev, (node))

/* GENERIC API with NAMED BASE structure */
#define mem_llist_nb_prepend(next, prev, base, ref, node) mem_llist_b_prepend(first, last, next, prev, (base), (ref), (node))
#define mem_llist_nb_append(next, prev, base, ref, node) mem_llist_b_append(first, last, next, prev, (base), (ref), (node))
#define mem_llist_nb_push(next, prev, base, node) mem_llist_b_push(first, last, next, prev, (base), (node))
#define mem_llist_nb_thrust(next, prev, base, node) mem_llist_b_thrust(first, last, next, prev, (base), (node))
#define mem_llist_nb_remove(next, prev, base, node) mem_llist_b_remove(first, last, next, prev, (base), (node))

/* NAMED GENERIC API with BASE structure */
#define mem_llist_bn_prepend(first, last, base, ref, node) mem_llist_b_prepend(first, last, next, prev, (base), (ref), (node))
#define mem_llist_bn_append(first, last, base, ref, node) mem_llist_b_append(first, last, next, prev, (base), (ref), (node))
#define mem_llist_bn_push(first, last, base, node) mem_llist_b_push(first, last, next, prev, (base), (node))
#define mem_llist_bn_thrust(first, last, base, node) mem_llist_b_thrust(first, last, next, prev, (base), (node))
#define mem_llist_bn_remove(first, last, base, node) mem_llist_b_remove(first, last, next, prev, (base), (node))

/* NAMED GENERIC API with NAMED BASE structure */
#define mem_llist_nbn_prepend(base, ref, node) mem_llist_b_prepend(first, last, next, prev, (base), (ref), (node))
#define mem_llist_nbn_append(base, ref, node) mem_llist_b_append(first, last, next, prev, (base), (ref), (node))
#define mem_llist_nbn_push(base, node) mem_llist_b_push(first, last, next, prev, (base), (node))
#define mem_llist_nbn_thrust(base, node) mem_llist_b_thrust(first, last, next, prev, (base), (node))
#define mem_llist_nbn_remove(base, node) mem_llist_b_remove(first, last, next, prev, (base), (node))

/* Add a new node previous to an existing node in a double linked list.
 * It inserts the element and returns the (new) first node in the list.
 * \list_first is the value which is returned by this function when \node is NOT
 * the new first node in the list.
 * \ref must point to a node in the list.
 * \ref_prev must point to the "prev" member of \ref.
 * \ref_prev_next must point to the "next" member of the node previous to \ref.
 * \node must point to a node which should be linked previous to \ref.
 * \node_next must point to the "next" member of \node.
 * \node_prev must point to the "prev" member of \node.
 */
static void *mem_llist_prepend(void *list_first,
                                      void *ref,
                                      void **ref_prev,
                                      void **ref_prev_next,
                                      void *node,
                                      void **node_next,
                                      void **node_prev)
{
    SUNDRY_ASSERT(ref && ref_prev && node && node_next && node_prev);
    SUNDRY_ASSERT(!*ref_prev || (ref_prev_next && *ref_prev_next == ref));
    SUNDRY_ASSERT(!*node_next && !*node_prev);

    *node_next = ref;
    *node_prev = *ref_prev;
    if(*ref_prev) *ref_prev_next = node;
    else return *ref_prev = node;
    *ref_prev = node;
    return list_first;
}
#define mem_llist_prepend(one, two, three, four, five, six, seven) \
    mem_llist_prepend((void*)(one), (void*)(two), (void**)(three), (void**)(four), (void*)(six), (void**)(seven), (void**)(eight))

/* Add a new node next to an existing node in a double linked list.
 * It inserts the element and returns the (new) last node in the list.
 * \list_last is the returned value when \node is NOT the new last node in the list.
 * \ref must point to a node in the list.
 * \ref_next must point to the "next" member of \ref.
 * \ref_next_prev must point to the "prev" member of the node next to \ref.
 * \node must point to a node which should be linked previous to \ref.
 * \node_next must point to the "next" member of \node.
 * \node_prev must point to the "prev" member of \node.
 */
static void *mem_llist_append(void *list_last,
                                     void *ref,
                                     void **ref_next,
                                     void **ref_next_prev,
                                     void *node,
                                     void **node_next,
                                     void **node_prev)
{
    SUNDRY_ASSERT(ref && ref_next && node && node_next && node_prev);
    SUNDRY_ASSERT(!*ref_next || (ref_next_prev && *ref_next_prev == ref));
    SUNDRY_ASSERT(!*node_next && !*node_prev);

    *node_prev = ref;
    *node_next = *ref_next;
    if(*ref_next) *ref_next_prev = node;
    else return *ref_next = node;
    *ref_next = node;
    return list_last;
}
#define mem_llist_append(one, two, three, four, five, six, seven) \
    mem_llist_append((void*)(one), (void*)(two), (void**)(three), (void**)(four), (void*)(six), (void**)(seven), (void**)(eight))

/* Adds a new node to the end of a list.
 * It returns always the new node.
 * \ref must point to the last member in the list.
 * \ref_next must point to the "next" member of \ref.
 * \node must point to a new node.
 * \node_next must point to the "next" member of \node.
 * \node_prev must point to the "prev" member of \node.
 */
static void *mem_llist_push(void *ref,
                                   void **ref_next,
                                   void *node,
                                   void **node_next,
                                   void **node_prev)
{
    SUNDRY_ASSERT((!ref && !ref_next) || (ref && ref_next && !*ref_next));
    SUNDRY_ASSERT(node && node_next && !*node_next && node_prev && !*node_prev);

    if(!ref) {
        *node_next = *node_prev = NULL;
        return node;
    }
    *node_prev = ref;
    *node_next = NULL;
    return *ref_next = node;
}
#define mem_llist_push(one, two, three, four, five) \
    mem_llist_push((void*)(one), (void**)(two), (void*)(three), (void**)(four), (void**)(five))

/* Adds a new node to the beginning of a list.
 * It returns always the new node.
 * \ref must point to the first member in the list.
 * \ref_prev must point to the "prev" member of \ref.
 * \node must point to a new node.
 * \node_next must point to the "next" member of \node.
 * \node_prev must point to the "prev" member of \node.
 */
static void *mem_llist_thrust(void *ref,
                                     void **ref_prev,
                                     void *node,
                                     void **node_next,
                                     void **node_prev)
{
    SUNDRY_ASSERT((!ref && !ref_prev) || (ref && ref_prev && !*ref_prev));
    SUNDRY_ASSERT(node && node_next && !*node_next && node_prev && !*node_prev);

    if(!ref) {
        *node_next = *node_prev = NULL;
        return node;
    }
    *node_prev = NULL;
    *node_next = ref;
    return *ref_prev = node;
}
#define mem_llist_thrust(one, two, three, four, five) \
    mem_llist_thrust((void*)(one), (void**)(two), (void*)(three), (void**)(four), (void**)(five))

/* Removes a node from a list.
 * It returns nothing.
 * \node is the node to be removed.
 * \node_next is the "next" member of \node.
 * \node_prev is the "prev" member of \node.
 * \prev_next is the "next" member of the previous node.
 * \next_prev is the "prev" member of the next node.
 */
static void mem_llist_remove(void *node,
                                    void **node_next,
                                    void **node_prev,
                                    void **prev_next,
                                    void **next_prev)
{
    SUNDRY_ASSERT(node && node_next && node_prev);
    SUNDRY_ASSERT(!prev_next || *prev_next == node);
    SUNDRY_ASSERT(!next_prev || *next_prev == node);

    if(prev_next) *prev_next = *node_next;
    if(next_prev) *next_prev = *node_prev;
    *node_next = *node_prev = NULL;
}
#define mem_llist_remove(one, two, three, four, five) \
    mem_llist_remove((void*)(one), (void**)(two), (void**)(three), (void**)(four), (void**)(five))


SUNDRY_EXTERN_C_END
#endif /* MEMORIA_INCLUDED_memoria_list_h */

