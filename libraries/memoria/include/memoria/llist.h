/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
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
 * \offset contains the offset of the mem_node_t in each node. \first is
 * a pointer to the first node in the linked list. If no node is present it
 * is NULL. \last points to the last member and is, in this case, also NULL.
 * The nodes are organized in a straight-lined linked list, thus the
 * previous element of the first element is NULL and the next element of the
 * last element is also NULL. \count contains the current number of elements
 * in the linked list and \match is the function which is used to compare two
 * elements. If \match is NULL, the elements' addresses are compared.
 */
typedef struct mem_list_t {
    size_t offset;
    struct mem_node_t *first;
    struct mem_node_t *last;
    size_t count;
    mem_match_t match;
} mem_list_t;

/* Single node of a linked list.
 * Your structure which you want to insert in a linked list must contain an object
 * of this structure. It is used to link it with the other elements.
 * You can iterate through the linked list yourself by accessing the ->next and ->prev
 * members. ->head is a link to the mem_ll_list structure.
 * Remember, the nodes are organized in a straight-lined linked list.
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
    ONS_ASSERT(list != NULL);

    list->offset = offset;
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
    list->match = NULL;
}

/* Returns true if the list has no nodes, otherwise false is returned.
 * You can get the number of elements by accessing the \count member of
 * a linked list.
 */
#define mem_ll_empty(list) ((list)->count == 0)

/* Compares the addresses \orig and \comp and returns an ons_comp_t. */
#define mem_ll_pmatch(orig, comp) (((comp) > (orig))?(ONS_GREATER):((((comp) < (orig))?(ONS_SMALLER):(ONS_EQUAL))))

/* Compares two nodes \orig and \comp of the list \list.
 * If no match function is set, mem_ll_pmatch() is called.
 * Returns an ons_comp_t value.
 */
#define mem_ll_match(list, orig, comp) (((list)->match)?((list)->match((orig), (comp))):(mem_ll_pmatch((orig), (comp))))

/* Returns a pointer to a real structure if you have only the mem_node_t pointer \node.
 * The macro takes as argument a type the void* pointer is casted to.
 */
#define MEM_LL_ENTRY(node, type) ((type)mem_ll_entry(node))
static inline void *mem_ll_entry(const mem_node_t *node) {
    ONS_ASSERT(node != NULL);

    return ((void*)node) - node->head->offset;
}

/* Links \raw_newm into the list \list in front of the already listed member \raw_member.
 * This does not allocate new memory, \raw_newm is simply linked into the list.
 * If \raw_member is the first node, \raw_newm becomes the first node.
 * Returns \raw_newm.
 */
static inline void *mem_ll_prepend(mem_list_t *list, void *raw_member, void *raw_newm) {
    mem_node_t *member, *newm;

    ONS_ASSERT(list != NULL);
    ONS_ASSERT(raw_member != NULL);
    ONS_ASSERT(raw_newm != NULL);

    member = raw_member + list->offset;
    newm = raw_newm + list->offset;

    newm->next = member;
    newm->prev = member->prev;
    if(member->prev) member->prev->next = newm;
    else list->first = newm;
    member->prev = newm;

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

    ONS_ASSERT(list != NULL);
    ONS_ASSERT(raw_member != NULL);
    ONS_ASSERT(raw_newm != NULL);

    member = raw_member + list->offset;
    newm = raw_newm + list->offset;

    newm->prev = member;
    newm->next = member->next;
    if(member->next) member->next->prev = newm;
    else list->last = newm;
    member->next = newm;

    ++list->count;

    return raw_newm;
}

/* Inserts \raw_newm as the first node into \list.
 * Returns \raw_newm.
 */
static inline void *mem_ll_thrust(mem_list_t *list, void *raw_newm) {
    mem_node_t *newm;

    ONS_ASSERT(list != NULL);
    ONS_ASSERT(raw_newm != NULL);

    if(!mem_ll_empty(list)) return mem_ll_prepend(list, list->first, raw_newm);
    else {
        newm = raw_newm + list->offset;
        list->first = list->last = newm;
        newm->next = newm->prev = NULL;
        ++list->count;
        return raw_newm;
    }
}

/* Inserts \raw_newm as the last node into \list.
 * Returns \raw_newm.
 */
static inline void *mem_ll_push(mem_list_t *list, void *raw_newm) {
    mem_node_t *newm;

    ONS_ASSERT(list != NULL);
    ONS_ASSERT(raw_newm != NULL);

    if(!mem_ll_empty(list)) return mem_ll_append(list, list->last, raw_newm);
    else {
        newm = raw_newm + list->offset;
        list->first = list->last = newm;
        newm->next = newm->prev = NULL;
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

    ONS_ASSERT(list != NULL);
    ONS_ASSERT(raw_member != NULL);

    member = raw_member + list->offset;
    if(list->count == 1) {
        list->count = 0;
        list->first = list->last = NULL;
        return raw_member;
    }
    else if(list->first == member) {
        list->first = member->next;
        list->first->prev = NULL;
    }
    else if(list->last == member) {
        list->last = member->prev;
        list->last->next = NULL;
    }
    else {
        member->prev->next = member->next;
        member->next->prev = member->prev;
    }

    --list->count;

    return raw_member;
}

/* Removes the first node in \list. If the list is empty, NULL is returned,
 * otherwise a pointer to the removed element is returned.
 */
static inline void *mem_ll_shift(mem_list_t *list) {
    ONS_ASSERT(list != NULL);

    if(mem_ll_empty(list)) return NULL;
    else return mem_ll_extract(list, ((void*)list->first) - list->offset);
}

/* Same as mem_ll_shift but removes the last node. */
static inline void *mem_ll_pop(mem_list_t *list) {
    ONS_ASSERT(list != NULL);

    if(mem_ll_empty(list)) return NULL;
    else return mem_ll_extract(list, ((void*)list->last) - list->offset);
}


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_llist_h */

