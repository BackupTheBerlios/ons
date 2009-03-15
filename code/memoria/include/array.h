/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* The array interface implements several memory structures
 * which can be accessed like an array.
 * They have no common ancestor, however, they are implemented
 * similarly so they are all in this header.
 *
 * Also "meta" arrays are declared here like the random number
 * generators.
 */


#ifndef ONS_INCLUDED_memoria_array_h
#define ONS_INCLUDED_memoria_array_h
ONS_EXTERN_C_BEGIN


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
 * \INITIAL_VAR is the number of elements which get allocated first.
 * \FREE specifies whether to shrink the allocated space if an element gets deleted and
 * \DOUBLE specifies whether \INITIAL_VAR should be doubled or simple added to the currently
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
 * Functions:
 *  - void ARR_NAME_init(ARR_NAME *array): Initializes the array.
 *  - void ARR_NAME_clear(ARR_NAME *array): Clears the array.
 *  - ELE_TYPE *ARR_NAME_push(ARR_NAME *array): Adds a new element at the tip.
 *  - void ARR_NAME_pop(ARR_NAME *array): Removes the tip.
 *  - ELE_TYPE *ARR_NAME_insert(ARR_NAME *array, mem_index_t index): Adds a new element at \index.
 *  - ELE_TYPE *ARR_NAME_thrust(ARR_NAME *array): Adds a new first element.
 *  - void ARR_NAME_remove(ARR_NAME *array, mem_index_t index): Removes the element at \index.
 *  - void ARR_NAME_shift(ARR_NAME *array): Removes the first element.
 */
#define MEM_ARRAY_DEFINE(ARR_NAME, ELE_TYPE, INITIAL_VAR, FREE, DOUBLE, FUNC_FREE) \
    typedef struct { \
        mem_index_t used; \
        mem_index_t size; \
        ELE_TYPE *list; \
    } ARR_NAME; \
    static void (* ARR_NAME##_free_func)(void*) = FUNC_FREE; \
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

static inline uint32_t mem_isaac_rand(mem_isaac_t *r) {
    ONS_ASSERT(r != NULL);

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

static inline uint32_t mem_rand(mem_rand_t *x) {
    ONS_ASSERT(x != NULL);

    uint32_t e = x->a - ONS_ROT(32, x->b, 27);
    x->a = x->b ^ ONS_ROT(32, x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

static inline void mem_seed(mem_rand_t *x, uint32_t seed) {
    ONS_ASSERT(x != NULL);

    uint32_t i;
    x->a = 0xf1ea5eed;
    x->b = x->c = x->d = seed;
    for(i = 0; i < 20; ++i) {
        mem_rand(x);
    }
}


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_array_h */

