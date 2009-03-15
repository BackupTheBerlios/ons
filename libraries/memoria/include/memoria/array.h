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

/* Dynamic length arrays.
 * This array implementation works like a stack. You allocate an array
 * structure and can push elements on top of the structure. You can
 * access all elements and pop the tip off.
 * This is really fast but most time you need functionality to insert
 * or delete elements in the middle of the array. We provide functions
 * which do that by either pushing one element to the top and moving
 * the stack upwards or by moving the stack downwards and poping the tip.
 * However, the indexes and pointers to the elements can change if you
 * remove or add one element in the middle of the array. This effect
 * does not occur if you only push/pop the elements.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_array_h
#define ONS_INCLUDED_memoria_array_h
ONS_EXTERN_C_BEGIN


#include <string.h>
#include <stdlib.h>


/** Template Dynamic Array
 ************************************************************************************************
 * This dynamic array implementation uses macros which define the functions which               *
 * operate on the array. You need to call MEM_ARRAY_DEFINE in your source or header             *
 * to define the functions which operate on the array. The advantage of this implementation     *
 * is that the base structure is really small. Furthermore you can access the elements          *
 * directly without any macro with the [] operator.                                             *
 * This makes this dynamic array implementation perfect for small arrays with probably only 10  *
 * elements.                                                                                    *
 ************************************************************************************************
 */

/* Defines a new array type.
 * Call this macro in your header or at the top of your source. \ARR_NAME will be the name
 * of the array structure, \ELE_TYPE is the element type which is saved in the array and
 * \INITIAL_VAR is the number of elements which get allocated first.
 * \FREE specifies whether to shrink the allocated space if an element gets deleted and
 * \DOUBLE specifies whether \INITIAL_VAR should be doubled or simple added to the currently
 * allocated elements.
 * \FUNC_FREE is a function of type \void (*func)(ELE_TYPE*) which is called on each element
 * before it is deleted. If NULL, nothing is done. The space where the element is located is
 * always freed by this implementation, but this function can be used if you store pointers
 * in this array and want to free the space the pointers point to before they get deleted.
 *
 * This macro defins the same functions as the generic dynamic array implementation. However,
 * they are prefixed with ARR_NAME instead of "mem_array".
 * Structure:
 *  - The structure contains 3 members:
 *    - \used: Contains the number of elements used.
 *    - \size: Contains the number of elements available.
 *    - \list: Pointer to the first member.
 * Functions:
 *  - void ARR_NAME_init(ARR_NAME *array): Initializes the array. (mem_array_init)
 *  - void ARR_NAME_clear(ARR_NAME *array): Clears the array. (mem_array_clear)
 *  - ELE_TYPE *ARR_NAME_push(ARR_NAME *array): Adds a new element at the tip. (mem_array_push)
 *  - void ARR_NAME_pop(ARR_NAME *array): Removes the tip. (mem_array_pop)
 *  - ELE_TYPE *ARR_NAME_insert(ARR_NAME *array, mem_index_t index): Adds a new element at \index. (mem_array_insert)
 *  - ELE_TYPE *ARR_NAME_thrust(ARR_NAME *array): Adds a new first element. (mem_array_thrust)
 *  - void ARR_NAME_remove(ARR_NAME *array, mem_index_t index): Removes the the element at \index. (mem_array_remove)
 *  - void ARR_NAME_shift(ARR_NAME *array): Removes the first element. (mem_array_shift)
 */
static inline void MEM_NULL(void *null) {
    return;
}
#define MEM_ARRAY_DEFINE(ARR_NAME, ELE_TYPE, INITIAL_VAR, FREE, DOUBLE, FUNC_FREE) \
    typedef struct ARR_NAME { \
        mem_index_t used; \
        mem_index_t size; \
        ELE_TYPE *list; \
    } ARR_NAME; \
    static void (* ARR_NAME##_free_func )(void*) = FUNC_FREE; \
    static inline void ARR_NAME##_init(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        array->used = 0; \
        array->size = 0; \
        array->list = NULL; \
    } \
    static inline void ARR_NAME##_clear(ARR_NAME *array) { \
        size_t i; \
        ONS_ASSERT(array != NULL); \
        if(ARR_NAME##_free_func != NULL) {for(i = 0; i < array->used; ++i) (ARR_NAME##_free_func)(&(array->list[i]));}; \
        array->used = array->size = 0; \
        mem_free(array->list); \
        array->list = NULL; \
    } \
    static inline ELE_TYPE *ARR_NAME##_push(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        if(array->used == array->size) { \
            if(DOUBLE) array->size = array->size ? (array->size << 1) : (INITIAL_VAR); \
            else array->size = array->size ? (array->size + INITIAL_VAR) : (INITIAL_VAR); \
            array->list = mem_realloc(array->list, array->size * sizeof(ELE_TYPE)); \
        } \
        return &array->list[array->used++]; \
    } \
    static inline void ARR_NAME##_pop2(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        mem_index_t size; \
        --array->used; \
        if(FREE) { \
            if(DOUBLE) size = array->size >> 1; \
            else size = array->size - INITIAL_VAR; \
            if(array->used <= size) { \
                if(size == 0) ARR_NAME##_clear(array); \
                else { \
                    array->list = mem_realloc(array->list, size * sizeof(ELE_TYPE)); \
                    array->size = size; \
                } \
            } \
        } \
    } \
    static inline void ARR_NAME##_pop(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        ONS_ASSERT(array->used); \
        if(ARR_NAME##_free_func != NULL) {(ARR_NAME##_free_func)(&(array->list[array->used - 1]));} \
        ARR_NAME##_pop2(array); \
    } \
    static inline ELE_TYPE *ARR_NAME##_insert(ARR_NAME *array, mem_index_t index) { \
        ONS_ASSERT(array != NULL); \
        if(index >= array->used || !ARR_NAME##_push(array)) return NULL; \
        memmove(array->list + index + 1, array->list + index, (array->used - index - 1) * sizeof(ELE_TYPE)); \
        return &array->list[index]; \
    } \
    static inline ELE_TYPE *ARR_NAME##_thrust(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        return ARR_NAME##_insert(array, 0); \
    } \
    static inline void ARR_NAME##_remove(ARR_NAME *array, mem_index_t index) { \
        ONS_ASSERT(array != NULL); \
        ONS_ASSERT(index < array->used); \
        ONS_ASSERT(array->used); \
        if(ARR_NAME##_free_func != NULL) {(ARR_NAME##_free_func)(&(array->list[array->used - 1]));} \
        memmove(array->list + index, array->list + index + 1, (array->used - index - 1) * sizeof(ELE_TYPE)); \
        ARR_NAME##_pop2(array); \
    } \
    static inline void ARR_NAME##_shift(ARR_NAME *array) { \
        ONS_ASSERT(array != NULL); \
        ARR_NAME##_remove(array, 0); \
    }


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_array_h */

