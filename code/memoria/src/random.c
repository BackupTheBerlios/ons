/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 28. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* This file is based on:
 * > isaac.c from:
 * > Copyright (C) 2007 Dominik Paulus and David Herrmann
 */

/* This file is based on rand.c by Bob Jenkins
 * ------------------------------------------------------------------------------
 * rand.c: By Bob Jenkins.  My random number generator, ISAAC.  Public Domain
 * Algorithm MODIFIED:
 *   960327: Creation (addition of randinit, really)
 *   970719: use context, not global variables, for internal state
 *   010626: Note this is public domain
 *   080104: Modified by Dominik Paulus to eliminate the weakness described by
 *           Jean-Philippe Aumasson.
 * ------------------------------------------------------------------------------
*/


#include "memoria/memoria.h"

/* *Truly* *random* seed value. */
mem_rand_t mem_grand = {0xdeabeaf, 0xdefcadad, 0xfea2ab1e, 0x73270017};

/* Rotates a 32bit value to the right. */
#define rotate_right_32(x, y) (((x) >> (y)) | ((x) << (32 - (y))))

/* Helper macros for ISAAC+. */
#define ind(mm, x) ((mm)[ (rotate_right_32(x, 2)) & (MEM_RANDSIZ-1) ])
#define rngstep(mix, a, b, mm, m, m2, r, x) \
{ \
    x = *m;  \
    a = (a ^ (mix)) + *(m2++); \
    *(m++) = y = ind(mm, x) + (a ^ b); \
    *(r++) = b = (ind(mm, rotate_right_32(y, MEM_RANDSIZL)) ^ a) + x; \
}

void mem_isaac_gen(mem_isaac_t *ctx) {
    register uint32_t a, b, x, y, *m, *mm, *m2, *r, *mend;

    ONS_ASSERT(ctx != NULL);

    mm = ctx->randmem;
    r = ctx->randrsl;
    a = ctx->randa;
    b = ctx->randb + (++ctx->randc);
    for(m = mm, mend = m2 = m + (MEM_RANDSIZ / 2); m < mend; ) {
        rngstep(a << 13, a, b, mm, m, m2, r, x);
        rngstep(a >> 6 , a, b, mm, m, m2, r, x);
        rngstep(a << 2 , a, b, mm, m, m2, r, x);
        rngstep(a >> 16, a, b, mm, m, m2, r, x);
    }
    for(m2 = mm; m2 < mend; ) {
        rngstep(a << 13, a, b, mm, m, m2, r, x);
        rngstep(a >> 6 , a, b, mm, m, m2, r, x);
        rngstep(a << 2 , a, b, mm, m, m2, r, x);
        rngstep(a >> 16, a, b, mm, m, m2, r, x);
    }
    ctx->randb = b;
    ctx->randa = a;
}

#define mix(a,b,c,d,e,f,g,h) \
{ \
    a^=b<<11; d+=a; b+=c; \
    b^=c>>2;  e+=b; c+=d; \
    c^=d<<8;  f+=c; d+=e; \
    d^=e>>16; g+=d; e+=f; \
    e^=f<<10; h+=e; f+=g; \
    f^=g>>4;  a+=f; g+=h; \
    g^=h<<8;  b+=g; h+=a; \
    h^=a>>9;  c+=h; a+=b; \
}

void mem_isaac_seed(mem_isaac_t *ctx) {
    signed int i;
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t *m, *r;

    ONS_ASSERT(ctx != NULL);

    ctx->randa = ctx->randb = ctx->randc = 0;
    m = ctx->randmem;
    r = ctx->randrsl;
    a = b = c = d = e = f = g = h = 0x9e3779b9;  /* the golden ratio */

    for(i = 0; i < 4; ++i) { /* scramble it */
        mix(a, b, c, d, e, f, g, h);
    }

    /* initialize using the contents of r[] as the seed */
    for(i = 0; i < MEM_RANDSIZ; i += 8) {
        a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
        e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
        mix(a, b, c, d, e, f, g, h);
        m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
        m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
    }
    /* do a second pass to make all of the seed affect all of m */
    for(i = 0; i < MEM_RANDSIZ; i += 8) {
        a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
        e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
        mix(a, b, c, d, e, f, g, h);
        m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
        m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
    }

    mem_isaac_gen(ctx); /* fill in the first set of results */
    ctx->randcnt = MEM_RANDSIZ; /* prepare to use the first set of results */
}

