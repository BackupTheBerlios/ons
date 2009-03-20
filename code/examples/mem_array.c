/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 20. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 20. March 2009
 */

/* This example shows how to use the array interface
 * of memoria. It shows an example for all types inside
 * of this interface.
 */


#include "include.h"
#include <memoria/memoria.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/* RNG tests
 * All random numbers are tested with an interface which generates one random
 * number, safes it into \safe and then generates as many random number until
 * this number occurs again.
 * This is not *real* test for randomness, but it shows how to use the generators.
 *
 * All RNGs generate 32bit random numbers, therefore, when 4,2 Billion numbers
 * are generated, the chance is 1:1 that the number in \safe will occur again.
 */


/* Contains the number we search for. */
uint32_t safe;
/* Is incremented everytime we generate the next number. */
unsigned long long i = 0;


/* Uses the simple/fast RNG interface with the global object.
 * This is called by \test_random.
 */
void echo_random() {
    uint32_t r = mem_rand(NULL);
    if(r == safe) {
        printf("One random number occured twice.\n");
        exit(1);
    }
    ++i;
    if(!(i % 100000000)) printf("%llu numbers generated; still not found\n", i);
}


/* Same as \echo_random but with global object. See \test_random2 for information
 * why to use it.
 */
mem_rand_t *r_obj;
void echo_random2() {
    uint32_t r = mem_rand(r_obj);
    if(r == safe) {
        printf("One random number occured twice.\n");
        exit(1);
    }
    ++i;
    if(!(i % 100000000)) printf("%llu numbers generated; still not found\n", i);
}


/* Uses the simple/fast RNG interface with teh global objects.
 * Uses \echo_random as worker function.
 *
 * Generates 1 Billion random numbers in 16seconds on my my Pentium IV 3.2GHz. (2009/03/20)
 */
void test_random() {
    /* Generate random numbers with the global non-threadsafe
     * random number object.
     */
    mem_seed(NULL, time(NULL));
    printf("Generating random numbers (seeded with the current time in seconds)\n");
    printf("First number is: %u (we exit when this number occurs the next time)\n", safe = mem_rand(NULL));
    printf("This can take really long if the RNG is *really* god, however, it can also take very short if you have luck ;)\n");
    /* \sample_call is defined in "include.h" and calls \echo_random endless times if the second parameter is 0. */
    sample_call(echo_random, 0);
}


/* Same as \test_random but uses an explicit RNG state object.
 * This allows to have two or more separat independant RNGs and
 * to call mem_rand() synchronously in two or more threads.
 *
 * Generates 1 Billion random numbers in 16seconds on my my Pentium IV 3.2GHz. (2009/03/20)
 */
void test_random2() {
    mem_rand_t obj;
    mem_seed(&obj, time(NULL));
    r_obj = &obj;
    printf("Generating random numbers (seeded with the current time in seconds)\n");
    printf("First number is: %u (we exit when this number occurs the next time)\n", safe = mem_rand(NULL));
    printf("This can take really long if the RNG is *really* god, however, it can also take very short if you have luck ;)\n");
    /* \sample_call is defined in "include.h" and calls \echo_random2 endless times if the second parameter is 0. */
    sample_call(echo_random2, 0);
}


/* Uses the isaac suite to test for randomness.
 * Works equally to \test_random2.
 *
 * Generates 1 Billion random numbers in 24seconds on my Pentium IV 3.2GHz. (2009/03/20)
 */
void test_isaac() {
    mem_isaac_t isaac;
    unsigned long long i;
    uint32_t r;

    /* \randrsl is used for the seed value. It has MEM_RANDSIZ members (probably 256) but we
     * initialize only the first because we have no further source for randomness.
     * The others can be set to zero or left uninitialized.
     */
    isaac.randrsl[0] = time(NULL);

    /* Uses isaac->randrsl to seed the RNG. */
    mem_isaac_seed(&isaac);

    for(i = 0; i < 10; ++i) {
        r = mem_isaac_rand(&isaac);
        printf("%u\n", r);
    }
}


/* Test the array suite by inserting/deleting objects and you have to check
 * with "htop" or another "task manager" whether the processes memory keeps stable
 * which indicates that there are hopefully no memory leaks.
 */
MEM_ARRAY_DEFINE(test_t, unsigned int, 4, 1, 1, NULL);
void test_array() {
    test_t arr;
    unsigned int i, *ele;
    unsigned int array[10];

    /* initialize */
    test_t_init(&arr);

    /* add new elements */
    ele = test_t_push(&arr);
    *ele = 5;

    *test_t_push(&arr) = 6;

    *test_t_insert(&arr, 0) = 100;

    /* remove elements */
    test_t_pop(&arr);

    test_t_remove(&arr, 0);

    /* merge a *real* array into the array */
    test_t_rmerge(&arr, array, 10, arr.used);

    /* Add lots more elements and remove them later. */
    for(i = 0; i < 1000; ++i) {
        test_t_push(&arr);
    }

    for(i = 0; i < 1000; ++i) {
        test_t_shift(&arr);
    }

    /* Clear the array. This removes all allocated space.
     * You can still push elements again which will cause new space to be allocated.
     *
     * However, you must clean the array if you do not need it anymore to free all space.
     * Otherwise, you will have memory leaks.
     */
    test_t_clear(&arr);
}


signed int main(signed int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: ./mem_array [option]\n");
        printf("    Options:\n");
        printf("    - rand: Tests the PRNG suite.\n");
        printf("    - isaac: Tests the CSPRNG suite.\n");
        printf("    - array: Tests the dynamic array (vector) suite.\n");
        return EXIT_FAILURE;
    }
    else if(0 == strcmp(argv[1], "rand")) test_random2();
    else if(0 == strcmp(argv[1], "isaac")) test_isaac();
    else if(0 == strcmp(argv[1], "array")) test_array();
    else {
        printf("Invalid parameter: '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

