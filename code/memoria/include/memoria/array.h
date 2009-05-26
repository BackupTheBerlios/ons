/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 3. April 2009
 */

/* The array interface implements several memory structures
 * which can be accessed like an array.
 * They have no common ancestor, however, they are implemented
 * similarly so they are all in this header.
 *
 * Also "meta" arrays are declared here like the random number
 * generators.
 */


#include <sundry/sundry.h>

#ifndef MEMORIA_INCLUDED_memoria_array_h
#define MEMORIA_INCLUDED_memoria_array_h
SUNDRY_EXTERN_C_BEGIN


#include <stdint.h>
#include <memoria/memoria.h>
#include <memoria/alloc.h>

/** Template Dynamic Array
 ************************************************************************************************
 * This dynamic array implementation uses macros which define the functions which               *
 * operate on the array. You need to call MEM_ARRAY_DEFINE in your source or header             *
 * to define the functions which operate on the array. The advantage of this implementation     *
 * is that the base structure is really small. Furthermore you can access the elements          *
 * directly, without any macro, with the [] operator.                                           *
 * This makes this dynamic array implementation perfect for small arrays with probably only 10  *
 * elements and also big arrays with thousands of elements.                                     *
 ************************************************************************************************
 */

/* Defines a new array type.
 * Call this macro in your header or at the top of your source. \ARR_NAME will be the name
 * of the array structure, \ELE_TYPE is the element type which is saved in the array and
 * \INITIAL_VAL is the number of elements which get allocated first.
 * \FREE specifies whether to shrink the allocated space if an element gets deleted and
 * \DOUBLE specifies whether \INITIAL_VAL should be doubled or simple added to the currently
 * allocated elements when new space needs to be allocated.
 * \FUNC_FREE is a function of type \void (*func)(ELE_TYPE*) which is called on each element
 * before it is deleted. If NULL, nothing is done. The space where the element is located is
 * always freed by this implementation, but this function can be used if you store pointers
 * in this array and want to free the space the pointers point to before they get deleted.
 *
 * Structure:
 *  - The structure contains 3 members:
 *    - \used: Contains the number of elements used.
 *    - \size: Contains the number of elements available.
 *    - \list: Pointer to the first member.
 *
 * Internal Functions:
 *   These functions are used by all other functions. You should never access them directly.
 *   Direct accessing these values can cause mem-leaks of you use them wrongly.
 *  - void ARR_NAME__resize(ARR_NAME *array, size_t size): Resizes the array to have \size elements.
 *  - ELE_TYPE *ARR_NAME__push(ARR_NAME *array): Adds one element to the top and returns a pointer to it.
 *  - void ARR_NAME__pop(ARR_NAME *array): Removes the top most element and shrink the array if required.
 *  - void ARR_NAME__free(ARR_NAME *array, size_t begin, size_t end): Frees the elements from \begin to \end (including).
 *
 * Base Functions:
 *   These functions implement the base tasks on an array. All other tasks can be emulated with
 *   these functions.
 *  - void ARR_NAME_init(ARR_NAME *array): Initializes the array.
 *  - void ARR_NAME_clear(ARR_NAME *array): Clears the array.
 *  - ELE_TYPE *ARR_NAME_push(ARR_NAME *array): Adds a new element at the tip.
 *  - void ARR_NAME_pop(ARR_NAME *array): Removes the tip.
 *  - void ARR_NAME_resize(ARR_NAME *array, size_t size): Resizes the array to \size.
 *
 * Additional Functions:
 *   These functions provide additional access to the array.
 *  - ELE_TYPE *ARR_NAME_insert(ARR_NAME *array, size_t index): Adds a new element at \index.
 *  - ELE_TYPE *ARR_NAME_thrust(ARR_NAME *array): Adds a new first element.
 *  - void ARR_NAME_remove(ARR_NAME *array, size_t index): Removes the element at \index.
 *  - void ARR_NAME_shift(ARR_NAME *array): Removes the first element.
 *  - void ARR_NAME_merge(ARR_NAME *array, const ARR_NAME *pnew, size_t index): Inserts all objects from \pnew into \array at \index.
 *  - void ARR_NAME_rmerge(ARR_NAME *array, const ELE_TYPE *pnew, size_t len, size_t index): Inserts \pnew (size = \len) into \array at \index.
 */
#define MEM_ARRAY_DEFINE(ARR_NAME, ELE_TYPE, INITIAL_VAL, FREE, DOUBLE, FUNC_FREE) \
    typedef struct { \
        size_t used; \
        size_t size; \
        ELE_TYPE *list; \
    } ARR_NAME; \
    static void (* ARR_NAME##_free_func)(ELE_TYPE*) = FUNC_FREE; \
    static void ARR_NAME##__resize(ARR_NAME *array, size_t size) { \
        size_t i, tmp; \
        SUNDRY_ASSERT(array != NULL); \
        if(size == 0) { \
            array->used = array->size = 0; \
            mem_free(array->list); \
            array->list = NULL; \
        } \
        else if(size == array->used) return; \
        else if(size <= array->size && (!FREE || size > ((DOUBLE)? (array->size >> 1): (array->size - INITIAL_VAL)))) { \
            array->used = size; \
        } \
        else { \
            for(i = (INITIAL_VAL)? (INITIAL_VAL): 1; i < size; ++i) { \
                if(i < ((DOUBLE)? (tmp = i << 1): (tmp = i + (INITIAL_VAL)))) i = tmp; \
                else i = size; \
            } \
            array->used = size; \
            array->size = i; \
            array->list = mem_realloc(array->list, array->size * sizeof(ELE_TYPE)); \
        } \
    } \
    static ELE_TYPE *ARR_NAME##__push(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        if(array->used == array->size) { \
            if(DOUBLE) array->size = array->size ? (array->size << 1) : (INITIAL_VAL); \
            else array->size += INITIAL_VAL; \
            array->list = mem_realloc(array->list, array->size * sizeof(ELE_TYPE)); \
        } \
        return &array->list[array->used++]; \
    } \
    static void ARR_NAME##__pop(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        SUNDRY_ASSERT(array->used != 0); \
        size_t size; \
        --array->used; \
        if(FREE) { \
            if(DOUBLE) size = array->size >> 1; \
            else size = array->size - INITIAL_VAL; \
            if(array->used <= size) { \
                if(size == 0) ARR_NAME##__resize(array, 0); \
                else { \
                    array->list = mem_realloc(array->list, size * sizeof(ELE_TYPE)); \
                    array->size = size; \
                } \
            } \
        } \
    } \
    static void ARR_NAME##__free(ARR_NAME *array, size_t begin, size_t end) { \
        size_t i; \
        SUNDRY_ASSERT(array != NULL); \
        SUNDRY_ASSERT(end >= begin); \
        SUNDRY_ASSERT(array->used >= end); \
        if(ARR_NAME##_free_func != NULL) { \
            for(i = begin; i <= end; ++i) ((ARR_NAME##_free_func)(&(array->list[i]))); \
        } \
    } \
    static void ARR_NAME##_init(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        array->used = 0; \
        array->size = 0; \
        array->list = NULL; \
    } \
    static void ARR_NAME##_clear(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        if(array->used > 0) ARR_NAME##__free(array, 0, array->used - 1); \
        ARR_NAME##__resize(array, 0); \
    } \
    static ELE_TYPE *ARR_NAME##_push(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        return ARR_NAME##__push(array); \
    } \
    static void ARR_NAME##_pop(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        SUNDRY_ASSERT(array->used > 0); \
        ARR_NAME##__free(array, array->used - 1, array->used - 1); \
        ARR_NAME##__pop(array); \
    } \
    static void ARR_NAME##_resize(ARR_NAME *array, size_t size) { \
        SUNDRY_ASSERT(array != NULL); \
        if(size < array->used) ARR_NAME##__free(array, size, array->used - 1); \
        ARR_NAME##__resize(array, size); \
    } \
    static ELE_TYPE *ARR_NAME##_insert(ARR_NAME *array, size_t index) { \
        SUNDRY_ASSERT(array != NULL); \
        SUNDRY_ASSERT(index <= array->used); \
        ARR_NAME##__push(array); \
        memmove(array->list + index + 1, array->list + index, (array->used - index) * sizeof(ELE_TYPE)); \
        return &array->list[index]; \
    } \
    static ELE_TYPE *ARR_NAME##_thrust(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        return ARR_NAME##_insert(array, 0); \
    } \
    static void ARR_NAME##_remove(ARR_NAME *array, size_t index) { \
        SUNDRY_ASSERT(array != NULL); \
        SUNDRY_ASSERT(index < array->used); \
        SUNDRY_ASSERT(array->used); \
        ARR_NAME##__free(array, index, index); \
        memmove(array->list + index, array->list + index + 1, (array->used - index - 1) * sizeof(ELE_TYPE)); \
        ARR_NAME##__pop(array); \
    } \
    static void ARR_NAME##_shift(ARR_NAME *array) { \
        SUNDRY_ASSERT(array != NULL); \
        ARR_NAME##_remove(array, 0); \
    } \
    static void ARR_NAME##_merge(ARR_NAME *array, const ARR_NAME *pnew, size_t index) { \
        SUNDRY_ASSERT(array != NULL && pnew != NULL && index <= array->used); \
        if(pnew->used == 0) return; \
        ARR_NAME##__resize(array, array->used + pnew->used); \
        memmove(array->list + index + pnew->used, array->list + index, (array->used - index) * sizeof(ELE_TYPE)); \
        memcpy(array->list + index, pnew->list, pnew->used * sizeof(ELE_TYPE)); \
    } \
    static void ARR_NAME##_rmerge(ARR_NAME *array, const ELE_TYPE *pnew, size_t len, size_t index) { \
        SUNDRY_ASSERT(array != NULL && pnew != NULL && index <= array->used); \
        if(len == 0) return; \
        ARR_NAME##__resize(array, array->used + len); \
        memmove(array->list + index + len, array->list + index, (array->used - index) * sizeof(ELE_TYPE)); \
        memcpy(array->list + index, pnew, len * sizeof(ELE_TYPE)); \
    }


/* Random Number Generators (RNGs).
 * This part implements an interface for non-cryptographic fast
 * RNGs (PRNGs) and cryptographic RNGs (CSPRNGs).
 * All RNGs are pseudo random number generators. If you want
 * to use "real" (nobody knows whether there is real randomness
 * out there) random numbers, you have to read /dev/random
 * or similar interfaces. Though, they can be REALLY slow and
 * even block.
 *
 * The CSPRNG provided here is ISAAC+. Developed originally by Bob Jenkins.
 * The PRNG provided here is the fast and smallest nonbiased PRNG found in
 * the internet. Both are dedicated to the Public Domain.
 */


/* The ISAAC+ implementation is originally from the first yaadns implementation ever.
 * It was copyrighted by:
 *   Copyright (C) 2007 Dominik Paulus and David Herrmann
 * It's dedicated to the Public Domain.
 *
 * ISAAC+ is an RNG developed by Bob Jenkins. The algorithm is dedicated to the Public Domain.
 * The original implementation is also dedicated to the public domain and modified by Dominik Paulus
 * to conform to the ISAAC+ specification.
 *
 * This file is based on rand.h by Bob Jenkins
 * ------------------------------------------------------------------------------
 * rand.h: definitions for a random number generator
 * By Bob Jenkins, 1996, Public Domain
 * ------------------------------------------------------------------------------
 */


/* ISAAC+ parameters. */
#define MEM_RANDSIZL 8 /* Use 8 for crypto, 4 for simulations */
#define MEM_RANDSIZ (1 << MEM_RANDSIZL)

/* Context of secure random number generator. */
typedef struct {
    uint32_t randcnt;
    uint32_t randrsl[MEM_RANDSIZ];
    uint32_t randmem[MEM_RANDSIZ];
    uint32_t randa;
    uint32_t randb;
    uint32_t randc;
} mem_isaac_t;

/* Uses the content of r->randrsl as seed value. */
extern void mem_isaac_seed(mem_isaac_t *r);

/* Generates MEM_RANDSIZ new numbers. */
extern void mem_isaac_gen(mem_isaac_t *r);

static uint32_t mem_isaac_rand(mem_isaac_t *r) {
    SUNDRY_ASSERT(r != NULL);

    if(!r->randcnt--) {
        mem_isaac_gen(r);
        r->randcnt = MEM_RANDSIZ - 1;
        return r->randrsl[r->randcnt];
    }
    else return r->randrsl[r->randcnt];
}

/* Fastest unbiased pseudo random number generator found
 * in the internet.
 * Developed by Bob Jenkins and works fast. THE HELL FAST!
 * It's almost twice as fast as ISAAC.
 */
typedef struct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} mem_rand_t;

extern mem_rand_t mem_grand;

static uint32_t mem_rand(mem_rand_t *x) {
    uint32_t e;
    if(x == NULL) x = &mem_grand;

    e = x->a - MEM_LROT(32, x->b, 27);
    x->a = x->b ^ MEM_LROT(32, x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

static void mem_seed(mem_rand_t *x, uint32_t seed) {
    uint32_t i;
    if(x == NULL) x = &mem_grand;

    x->a = 0xf1ea5eed;
    x->b = x->c = x->d = seed;
    for(i = 0; i < 20; ++i) {
        mem_rand(x);
    }
}


SUNDRY_EXTERN_C_END
#endif /* MEMORIA_INCLUDED_memoria_array_h */

