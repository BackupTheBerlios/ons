/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 22. February 2009
 */

/* Linked lists.
 * Most applications use their own linked lists and, of course, every
 * C developer must be able to implement it's own linked lists. However,
 * to ease the task of adding and removing elements, this header provides
 * macros for linked lists.
 *
 * Function names:
 * - FIRST: Returns first member.
 * - LAST: Returns last member.
 * - PREPEND: Insert before a member.
 * - APPEND: Insert after a member.
 * - THRUST: Insert as first member.
 * - PUSH: Insert as last member.
 * - REMOVE / EXTRACT: Remove member.
 * - SHIFT: Remove first member.
 * - POP: Remove last member.
 * - SORT: Sorts the whole list.
 * - SRTD_PREPEND: Sorted prepend.
 * - SRTD_APPEND: Sorted append.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_llist_h
#define ONS_INCLUDED_memoria_llist_h
ONS_EXTERN_C_BEGIN


#include <memoria/mem.h>

/* Function prototype.
 * If the linked list is sorted, a function of this type is called if two
 * elements are compared. If \comparison is greater than \orig, return
 * ONS_GREATER, if equal return ONS_EQUAL, and if smaller return ONS_SMALLER.
 * Your own prototype should substitute "void*" with the datatype which is
 * used in your linked list.
 */
typedef ons_comp_t (*mem_match_t)(const void *orig, const void *comparison);

/* Forward declarations. */
struct mem_list_t;
struct mem_node_t;

/* Linked list.
 * This is the base structure of the linked list.
 * \offset contains the offset of the mem_node_t in each node. \nodes is
 * a pointer to the first node in the linked list. If no node is present it
 * is NULL. The nodes are organized in a circular linked list, thus the
 * previous element of the first element is the last element. \count contains
 * the current number of elements in the linked list and \match is the function
 * which is used to compare two elements. If \match is NULL, the elements'
 * addresses are compared.
 */
typedef struct mem_list_t {
    size_t offset;
    struct mem_node_t *nodes;
    size_t count;
    mem_match_t match;
} mem_list_t;

/* Single node of a linked list.
 * Your structure which you want to insert in a linked list must contain an object
 * of this structure. It is used to link it with the other elements.
 * You can iterate through the linked list yourself by accessing the ->next and ->prev
 * members. ->head is a link to the mem_ll_list structure.
 * Remember, the nodes are organized in a circular linked list.
 */
typedef struct mem_node_t {
    struct mem_node_t *next;
    struct mem_node_t *prev;
    struct mem_list_t *head;
} mem_node_t;

/* Initializes a linked list structure.
 * Takes as first argument \list a pointer to the linked list. \offset is the offset
 * of the mem_node_t object in the structure you want to save in the linked list.
 * To ease the use you can use the MEM_LL_INIT macro which has the same effect
 * but takes as first argument a pointer to the list, as second argument the type of
 * the structure which you want to save in the linked list and as third argument the
 * name of the mem_node_t member in this structure.
 */
#define MEM_LL_INIT(list, type, name) mem_ll_init((list), offsetof(type, name))
static inline void mem_ll_init(mem_list_t *list, size_t offset) {
    assert(list != NULL);

    list->offset = offset;
    list->nodes = NULL;
    list->count = 0;
    list->match = NULL;
}

/* Returns true if the list has no nodes, otherwise false is returned.
 * You can get the number of elements by accessing the \count member of
 * a linked list.
 */
#define mem_ll_empty(list) (assert(list != NULL), (list)->count == 0)

/* Compares the addresses \orig and \comp and returns an ons_comp_t. */
#define mem_ll_pmatch(orig, comp) (((comp) > (orig))?(ONS_GREATER):((((comp) < (orig))?(ONS_SMALLER):(ONS_EQUAL))))

/* Compares two nodes \orig and \comp of the list \list.
 * If no match function is set, mem_ll_pmatch() is called.
 * Returns an ons_comp_t value.
 */
#define mem_ll_match(list, orig, comp) (((list)->match)?((list)->match((orig), (comp))):(mem_ll_pmatch((orig), (comp))))

/* Returns a pointer to a real structure if you have only the mem_node_t pointer \node.
 * The macro takes as argument a cast the void* pointer is casted to.
 */
#define MEM_LL_ENTRY(node, type) ((type)mem_ll_entry(node))
static inline void *mem_ll_entry(const mem_node_t *node) {
    assert(node != NULL);

    return ((void*)node) - node->head->offset;
}

/* Returns a pointer to the first element of the linked list. If the list
 * \list is empty, NULL is returned.
 */
static inline void *mem_ll_first(const mem_list_t *list) {
    assert(list != NULL);

    if(!mem_ll_empty(list)) return ((void*)list->nodes) - list->offset;
    else return NULL;
}

/* Returns a pointer to the last element of the linked list. If the list \list is empty
 * NULL is returned.
 */
static inline void *mem_ll_last(const mem_list_t *list) {
    assert(list != NULL);

    if(!mem_ll_empty(list)) return ((void*)list->nodes->prev) - list->offset;
    else return NULL;
}

/* Links \raw_newm into the list \list in front of the already listed member \raw_member.
 * This does not allocate new memory, \raw_newm is simply linked into the list.
 * If \raw_member is the first node, \raw_newm becomes the first node.
 * Returns \raw_newm.
 */
static inline void *mem_ll_prepend(mem_list_t *list, void *raw_member, void *raw_newm) {
    mem_node_t *member, *newm;

    assert(list != NULL);
    assert(raw_member != NULL);
    assert(raw_newm != NULL);

    member = raw_member + list->offset;
    newm = raw_newm + list->offset;

    newm->next = member;
    newm->prev = member->prev;
    member->prev->next = newm;
    member->prev = newm;

    if(list->nodes == member) list->nodes = newm;
    ++list->count;

    return raw_newm;
}

/* Links \raw_newm into the list \list directly after the already listed member \raw_member.
 * This does not allocate new memory, \raw_newm is simply linked into the list.
 * If \raw_member is the last node, \raw_newm becomes the last node.
 * Returns \raw_newm.
 */
static inline void *mem_ll_append(mem_list_t *list, void *raw_member, void *raw_newm) {
    mem_node_t *member, *newm;

    assert(list != NULL);
    assert(raw_member != NULL);
    assert(raw_newm != NULL);

    member = raw_member + list->offset;
    newm = raw_newm + list->offset;

    newm->prev = member;
    newm->next = member->next;
    member->next->prev = newm;
    member->next = newm;

    ++list->count;

    return raw_newm;
}

/* Inserts \raw_newm as the first node into \list.
 * Returns \raw_newm.
 */
static inline void *mem_ll_thrust(mem_list_t *list, void *raw_newm) {
    mem_node_t *newm;

    assert(list != NULL);
    assert(raw_newm != NULL);

    if(!mem_ll_empty(list)) return mem_ll_prepend(list, list->nodes, raw_newm);
    else {
        newm = raw_newm + list->offset;
        list->nodes = newm->next = newm->prev = newm;
        ++list->count;
        return raw_newm;
    }
}

/* Inserts \raw_newm as the last node into \list.
 * Returns \raw_newm.
 */
static inline void *mem_ll_push(mem_list_t *list, void *raw_newm) {
    mem_node_t *newm;

    assert(list != NULL);
    assert(raw_newm != NULL);

    if(!mem_ll_empty(list)) return mem_ll_append(list, list->nodes->prev, raw_newm);
    else {
        newm = raw_newm + list->offset;
        list->nodes = newm->next = newm->prev = newm;
        ++list->count;
        return raw_newm;
    }
}

/* Removes \raw_member from the linked list \list. It does not
 * free any memory of \raw_member. You have to delete the object
 * yourself.
 * Returns \raw_member.
 */
#define mem_ll_extract(list, raw_member) mem_ll_remove((list), (raw_member))
static inline void *mem_ll_remove(mem_list_t *list, void *raw_member) {
    mem_node_t *member;

    assert(list != NULL);
    assert(raw_member != NULL);

    member = raw_member + list->offset;
    if(member->next == member) {
        list->count = 0;
        return raw_member;
    }
    member->prev->next = member->next;
    member->next->prev = member->prev;

    if(list->nodes == member) list->nodes = member->next;
    --list->count;

    return raw_member;
}

/* Removes the first node in \list. If the list is empty, NULL is returned,
 * otherwise a pointer to the removed element is returned.
 */
static inline void *mem_ll_shift(mem_list_t *list) {
    assert(list != NULL);

    if(mem_ll_empty(list)) return NULL;
    else return mem_ll_extract(list, ((void*)list->nodes) - list->offset);
}

/* Same as mem_ll_shift but removes the last node. */
static inline void *mem_ll_pop(mem_list_t *list) {
    assert(list != NULL);

    if(mem_ll_empty(list)) return NULL;
    else return mem_ll_extract(list, ((void*)list->nodes->prev) - list->offset);
}

/* Sorts the list.
 * This sorts all nodes in the list \list with the \match function set
 * in \list. If \match is NULL, the addresses are compared.
 * Sorting is always done ascending, thus, the first element is the
 * smallest and the last the greatest.
 */
static inline void mem_ll_sort(mem_list_t *list) {
    mem_node_t *iter, *iiter, *ilist;

    assert(list != NULL);

    /* Sorting possible? */
    if(mem_ll_empty(list) || list->nodes->next == list->nodes) return;

    /* Create new temporary sorted list. */
    ilist = list->nodes;
    list->nodes->prev->next = list->nodes->next;
    list->nodes->next->prev = list->nodes->prev;
    list->nodes = list->nodes->next;
    ilist->next = ilist->prev = ilist;

    /* Sort every member into this list. */
    while(list->nodes) {
        /* remove from llist */
        iter = list->nodes;
        if(iter->next != iter) {
            list->nodes->prev->next = list->nodes->next;
            list->nodes->next->prev = list->nodes->prev;
            list->nodes = iter->next;
        }
        else list->nodes = NULL;

        /* insert into new llist */
        iiter = ilist;
        do {
            if(mem_ll_match(list, ((void*)iiter) - list->offset, ((void*)iter) - list->offset) != ONS_GREATER) {
                iter->next = iiter;
                iter->prev = iiter->prev;
                iiter->prev->next = iter;
                iiter->prev = iter;
                if(ilist == iiter) ilist = iter;
                goto next_round;
            }
            iiter = iiter->next;
        } while(iiter != ilist);

        /* Not inserted, yet, thus, add it at tail. */
        iter->next = ilist;
        iter->prev = ilist->prev;
        ilist->prev->next = iter;
        ilist->prev = iter;

        /* insert next member */
        next_round:
        continue;
    }
    list->nodes = ilist;
}

/* Inserts an element into a sorted list. If other elements are equal to this one, it is
 * inserted before them. Returns \raw_member.
 * Sorting is always done ascending.
 */
static inline void *mem_ll_srtd_prepend(mem_list_t *list, void *raw_member) {
    mem_node_t *iter;

    assert(list != NULL);
    assert(raw_member != NULL);

    if(mem_ll_empty(list)) return mem_ll_thrust(list, raw_member);

    iter = list->nodes;
    do {
        if(mem_ll_match(list, ((void*)iter) - list->offset, raw_member) != ONS_GREATER) return mem_ll_prepend(list, ((void*)iter) - list->offset, raw_member);
        iter = iter->next;
    } while(iter != list->nodes);

    return mem_ll_push(list, raw_member);
}

/* Same as mem_ll_srtd_prepend() but inserts the element after all equal elements. */
static inline void *mem_ll_srtd_append(mem_list_t *list, void *raw_member) {
    mem_node_t *iter;

    assert(list != NULL);
    assert(raw_member != NULL);

    if(mem_ll_empty(list)) return mem_ll_thrust(list, raw_member);

    iter = list->nodes;
    do {
        if(mem_ll_match(list, ((void*)iter) - list->offset, raw_member) == ONS_SMALLER) return mem_ll_prepend(list, ((void*)iter) - list->offset, raw_member);
        iter = iter->next;
    } while(iter != list->nodes);

    return mem_ll_push(list, raw_member);
}


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_llist_h */

