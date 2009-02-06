/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
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


/** Generic Dynamic Array
 ****************************************************************************************
 * The first array implementation uses a structure which can be used with any type      *
 * and without any macros. It allows optimization through options but has a huge        *
 * base structure. If it is used with hundreds of elements, this doesn't matter but     *
 * if you want to save only 10 small elements, the array-base structure would probably  *
 * be bigger than the elements. In this case you should look at the second array        *
 * implementation below this one.                                                       *
 ****************************************************************************************
 */

#include <memoria/mem.h>

/* Dynamic array.
 * Create such a structure on the stack/heap and initialize it with the mem_array_init()
 * function. After this you can push/pop elements with the provided functions.
 * The \used member contains always the number of elements which are currently in the array.
 * The pointer to the first element is \list. \list can be NULL if \used is 0, otherwise it
 * must be a valid address.
 * You should not need to access the other elements directly, however, I describe them here, too.
 * The real size of the array is in \size. \reserve contains the padding which is used when
 * reallocating new space. \element contains the size of a single element.
 * \opt_free and \opt_double are options which can be set (1) or unset (0). See mem_array_init()
 * for their describtion.
 */
typedef struct mem_array_t {
    mem_index_t reserve;            /* Number of elements we allocate in one malloc() call. */
    mem_index_t size;               /* Current number of available elements. */
    mem_index_t used;               /* Current number of used elements. */
    size_t element;                 /* Size of one element. */
    void *list;                     /* Pointer to the allocated array. */

    unsigned int opt_free : 1;      /* If true, buffer is also freed if not used, anymore. */
    unsigned int opt_double : 1;    /* If true, "reserve" is not used and the size is doubled on a realloc(). */
} mem_array_t;

/** This array works with arbitrary datatypes. Therefore it is not possible to access the array
 * with the [] brackets. However, the same effect is possible with the following macros. They
 * simply multiply the element size with the index.
 */
/* This macro returns the index of a pointer to an element. \array must point to the array structure
 * and \ele to the element. Returns a mem_index_t.
 */
#define MEM_ARRAY_INDEX(array, ele) ((mem_index_t)(((void*)(ele)) - ((array)->list)))
/* This macro returns a pointer to the element at index \index of array \array. */
#define MEM_ARRAY(array, index) ((array)->list+(index)*(array)->element)
/* This macro returns the pointer to the element at index \index of array \array and
 * casts the returned pointer to \type.
 */
#define MEM_TYPE(array, index, type) ((type)MEM_ARRAY((array), (index)))
/* Same as MEM_TYPE but \type is the dereferenced type. Thus, the dereferenced value is returned. */
#define MEM_DEREF(array, index, type) (*(type*)MEM_ARRAY((array), (index)))

/* Initializes an allocated array structure.
 * This should be called on every array you created, before accessing the array. \array should point to
 * the array, \element should contain the size of the element you want to store in this array and \reserve
 * is the padding which is used in allocation.
 * The array allocates memory from the heap with \reserve elements. If you saved \reserve elements, it
 * reallocates the memory plus \reserve elements so you can store additional elements. If you set \reverse
 * to 1, it must reallocate the memory at every push (very slow). If you set \reserve to a very high value,
 * much memory is used unnecessarily.
 * After you initialized the array you can set options on the array: (You can change the options at any time!)
 * ->opt_free: If you delete elements in the array and the free unused space gets bigger than \reserve, the
 *             memory will be reallocated to a smaller value if this option is set. Otherwise the memory
 *             will never get smaller.
 * ->opt_double: Instead of adding \reserve elements to the array when reallocating, it now allocates
 *               \reserve elements first and then doubles or halves the number of elements on every
 *               reallocating. This decreases the number of allocations but probably increases the number
 *               of unused space.
 * I recommend either setting both options or none.
 * If \reserve or \element are 0, they are set to 1.
 */
static inline void mem_array_init(struct mem_array_t *array, size_t element, mem_index_t reserve) {
    if(reserve == 0) reserve = 1;
    if(element == 0) element = 1;

    array->list = NULL;
    array->reserve = reserve;
    array->size = 0;
    array->used = 0;
    array->element = element;
    array->opt_free = 0;
    array->opt_double = 0;
}

/* Removes all elements of the array \array and frees all allocated space.
 * If you add again elements to the array, new space will be allocated.
 * If you want to use the \list array still but do not need the array functionality
 * you can simply use the pointer in \list and delete the mem_array_t object yourself
 * without calling this function.
 */
static inline void mem_array_clean(struct mem_array_t *array) {
    mem_free(array->list);
    array->list = NULL;
    array->size = 0;
    array->used = 0;
}

/* Helper function: Computes the value of \size which is used if the memory is increased. */
#define _mem_array_inc_size(array) ((array->opt_double)?(array->size << 1):(array->size + array->reserve))
/* Helper function: Computes the value of \size which is used if the memory is decreased. */
#define _mem_array_dec_size(array) ((array->opt_double)?(array->size >> 1):(array->size - array->reserve))

/* Pushes a new element at the top of the array.
 * \array should point to an initialized array. This function
 * returns a pointer to the new element or NULL if memory allocation failed.
 * Pushing is the fastest way to insert new elements.
 */
static inline void *mem_array_push(struct mem_array_t *array) {
    void *tmp;
    mem_index_t size;
    if(array->size == 0) {
        array->list = mem_malloc(array->reserve * array->element);
        if(!array->list) return NULL;
        array->size = array->reserve;
    }
    else if(array->used == array->size) {
        size = _mem_array_inc_size(array);
        tmp = mem_realloc(array->list, size * array->element);
        if(!tmp) return NULL;
        array->list = tmp;
        array->size = size;
    }

    /* Return pointer and increase \used. */
    return array->list + array->used++ * array->element;
}

/* Removes the tip.
 * This is the fastest way to remove an element. It removes the most top
 * element in the array.
 * The other elements are not changed.
 */
static inline void mem_array_pop(struct mem_array_t *array) {
    void *tmp;
    mem_index_t size;

    --array->used;

    if(array->opt_free) {
        size = _mem_array_dec_size(array);
        if(array->used <= size) {
            if(size == 0) mem_array_clean(array);
            else {
                tmp = mem_realloc(array->list, size * array->element);
                /* Reallocation should never fail if the size is smaller as before.
                 * However, if it fails, just leave it as it is.
                 */
                if(tmp) {
                    array->list = tmp;
                    array->size = size;
                }
            }
        }
    }
}

/* Inserts a new element at position \index.
 * This pushes an element at the top and moves every element beyond \index (including \index)
 * one element further. Thus, all elements with an index greater or equal than \index
 * get an increased index and address.
 * This memory move is much slower than pushing an element at the top.
 * The function returns a pointer to the new element or NULL if memory allocation failed or
 * \index is an invalid index.
 */
static inline void *mem_array_insert(struct mem_array_t *array, mem_index_t index) {
    if(index >= array->used || !mem_array_push(array)) return NULL;
    memmove(array->list + array->element * (index + 1),
            array->list + array->element * index,
            (array->used - index - 1) * array->element);
    return array->list + index * array->element;
}

/* Inserts a new element at the base of the array. */
#define mem_array_thrust(array) mem_array_insert((array), 0)

/* Removes one element of the array.
 * This moves all elements with an index greater than \index in the array \array
 * one element down. All these elements get an decreased index and address.
 * After that, the tip is poped.
 */
static inline void mem_array_remove(struct mem_array_t *array, mem_index_t index) {
    if(array->used == 0 || index >= array->used) return;
    memmove(array->list + array->element * index,
            array->list + array->element * (index + 1),
            (array->used - index - 1) * array->element);
    mem_array_pop(array);
}

/* Removes the first element of the array. */
#define mem_array_shift(array) mem_array_remove((array), 0)



/** Template Dynamic Array
 ************************************************************************************************
 * The second dynamic array implementation uses macros which define the functions which         *
 * operate on the array. You need to call MEM_ARRAY_DECLARE in your source or header            *
 * to declare the functions which operate on the array. The advantage of this implementation    *
 * is that the base structure is really small. Furthermore you can access the elements          *
 * directly without any macro with the [] operator.                                             *
 * But to keep this array small and fast there is no way to change the options of the array.    *
 * The array does never free() any memory and doubles the size of the padding at every          *
 * reallocation. free() is only called if you clear the whole array. You can still add new      *
 * elements after clearing it, new space will be allocated. This makes this dynamic array       *
 * implementation perfect for small arrays with probably only 10 elements.                      *
 ************************************************************************************************
 */

/* Declares a new array type.
 * Call this macro in your header or at the top of your source. \ARR_NAME will be the name
 * of the array structure, \ELE_TYPE is the element type which is saved in the array and
 * \INITIAL_VAR is the number or \elements which get allocated first.
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
 *  - void ARR_NAME_clean(ARR_NAME *array): Cleans the array. (mem_array_clean)
 *  - ELE_TYPE *ARR_NAME_push(ARR_NAME *array): Adds a new element at the tip. (mem_array_push)
 *  - void ARR_NAME_pop(ARR_NAME *array): Removes the tip. (mem_array_pop)
 *  - ELE_TYPE *ARR_NAME_insert(ARR_NAME *array, mem_index_t index): Adds a new element at \index. (mem_array_insert)
 *  - ELE_TYPE *ARR_NAME_thrust(ARR_NAME *array): Adds a new first element. (mem_array_thrust)
 *  - void ARR_NAME_remove(ARR_NAME *array, mem_index_t index): Removes the the element at \index. (mem_array_remove)
 *  - void ARR_NAME_shift(ARR_NAME *array): Removes the first element. (mem_array_shift)
 */
#define MEM_ARRAY_DECLARE(ARR_NAME, ELE_TYPE, INITIAL_VAR) \
    typedef struct ARR_NAME { \
        mem_index_t used; \
        mem_index_t size; \
        ELE_TYPE *list; \
    } ARR_NAME; \
    static inline void ARR_NAME##_init(ARR_NAME *array) { \
        array->used = 0; \
        array->size = 0; \
        array->list = NULL; \
    } \
    static inline void ARR_NAME##_clean(ARR_NAME *array) {\
        array->used = array->size = 0;\
        mem_free(array->list);\
        array->list = NULL;\
    } \
    static inline ELE_TYPE *ARR_NAME##_push(ARR_NAME *array) { \
        ELE_TYPE *tmp; \
        mem_index_t nsize; \
        if(array->used == array->size) { \
            nsize = array->size ? (array->size << 1) : (INITIAL_VAR); \
            tmp = mem_realloc(array->list, nsize * sizeof(ELE_TYPE)); \
            if(!tmp) return NULL; \
            array->size = nsize; \
            array->list = tmp; \
        } \
        return &array->list[array->used++]; \
    } \
    static inline void ARR_NAME##_pop(ARR_NAME *array) { \
        --array->used; \
    } \
    static inline ELE_TYPE *ARR_NAME##_insert(ARR_NAME *array, mem_index_t index) { \
        if(index >= array->used || !ARR_NAME##_push(array)) return NULL; \
        memmove(array->list + index + 1, array->list + index, (array->used - index - 1) * sizeof(ELE_TYPE)); \
        return &array->list[index]; \
    } \
    static inline ELE_TYPE *ARR_NAME##_thrust(ARR_NAME *array) { \
        return ARR_NAME##_insert(array, 0); \
    } \
    static inline void ARR_NAME##_remove(ARR_NAME *array, mem_index_t index) { \
        if(array->used == 0 || index >= array->used) return; \
        memmove(array->list + index, array->list + index + 1, (array->used - index - 1) * sizeof(ELE_TYPE)); \
        ARR_NAME##_pop(array); \
    } \
    static inline void ARR_NAME##_shift(ARR_NAME *array) { \
        ARR_NAME##_remove(array, 0); \
    }


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_array_h */

