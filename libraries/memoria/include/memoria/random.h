/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 28. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Random Number Generators (RNGs).
 * This file provides an interface for non-cryptographic fast
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

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_random_h
#define ONS_INCLUDED_memoria_random_h
ONS_EXTERN_C_BEGIN


/* The ISAAC implementation is originally from the first yaadns implementation ever.
 * It was copyrighted by:
 *   Copyright (C) 2007 Dominik Paulus and David Herrmann
 * It's dedicated to the Public Domain.
 *
 * ISAAC is an RNG developed by Bob Jenkins. The algorithm is dedicated to the Public Domain.
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
typedef struct mem_isaac_t {
    uint32_t randcnt;
    uint32_t randrsl[MEM_RANDSIZ];
    uint32_t randmem[MEM_RANDSIZ];
    uint32_t randa;
    uint32_t randb;
    uint32_t randc;
} mem_isaac_t;

/* Uses the content of r->randrsl as seed value. */
void mem_isaac_seed(mem_isaac_t *r);

/* Generates MEM_RANDSIZ new numbers. */
void mem_isaac_gen(mem_isaac_t *r);

static inline uint32_t mem_isaac_rand(mem_isaac_t *r) {
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
typedef struct mem_rand_t {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} mem_rand_t;

static inline uint32_t mem_rand(mem_rand_t *x) {
    uint32_t e = x->a - ONS_ROT(32, x->b, 27);
    x->a = x->b ^ ONS_ROT(32, x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

static inline void mem_seed(mem_rand_t *x, uint32_t seed) {
    uint32_t i;
    x->a = 0xf1ea5eed;
    x->b = x->c = x->d = seed;
    for(i = 0; i < 20; ++i) {
        mem_rand(x);
    }
}


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_random_h */

