/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 2. January 2009
 */

/* Main source file of ONS.
 * This file implements main functionality used in every library which is
 * part of ONS.
 * Only really crucial functions/variables are implemented here.
 */

#include "memoria/memoria.h"
#include <stdlib.h>
#include <stdio.h>

/* The current handler called if memory allocation failed.
 * If it is NULL no handler is called.
 */
void (*mem_outofmem)(void) = NULL;

/* Include mmap() header if MAP_ANONYMOUS is available. */
#if defined(ONS_CONF_HAVE_MMAP) && defined(MAP_ANONYMOUS)
    #include <sys/mman.h>
#endif

/* Returns a new zero-initialized block either from mmap() or,
 * if not present, from malloc.
 */
void *mem_block_alloc(size_t size) {
#if defined(ONS_CONF_HAVE_MMAP) && defined(MAP_ANONYMOUS)
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED) {
        if(mem_outofmem) mem_outofmem();
        return NULL;
    }
    else return ptr;
#else
    return mem_zmalloc(size);
#endif
}

/* Frees a block allocated with mem_block_alloc().
 * If NULL nothing is done.
 */
void mem_block_free(void *ptr, size_t size) {
#if defined(ONS_CONF_HAVE_MMAP) && defined(MAP_ANONYMOUS)
    if(ptr) munmap(ptr, size);
#else
    mem_free(ptr);
#endif
}

/* The sample outofmem handler. It sets an internal state
 * to terminate the process only when the handler is called
 * twice.
 * This mechanism allows to call cleanup routines but prevents
 * endless loops when cleanup routines allocate memory themself.
 */
/** In multithreaded environments there can be two threads at
 * the same time in this handler. There is no case in which this
 * could lead to unknown behaviour.
 */
void mem_outofmem_handler(void) {
    static int called = 0;

    if(called) {
        fprintf(stderr, "Memory allocation failed, exiting...");
        abort();
    }
    called = 1;
    fprintf(stderr, "Memory allocation failed, calling cleanup routines...");
}

