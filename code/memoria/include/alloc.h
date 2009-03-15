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

/* This interface wraps the traditional malloc()'ish functions
 * and modifies them to never return NULL. If the memory allocation
 * fails, a global handler is called which must not return.
 */


#ifndef ONS_INCLUDED_memoria_alloc_h
#define ONS_INCLUDED_memoria_alloc_h
ONS_EXTERN_C_BEGIN


#include <stdlib.h>
#include <string.h>

/* Set this variable to the out-of-mem handler.
 * It is initialized with NULL and if it is NULL, no out-of-mem
 * handler is called.
 * Please be aware that the handler can be called parallel in
 * multithreaded programs.
 *
 * This handler should never return. If it returns, libmemoria
 * will abort with a default error message.
 */
extern void (*mem_outofmem)(void);

/* This macro checks whether the given value is NULL.
 * If it is not NULL, it returns, otherwise it calls the global
 * out-of-mem handler if it is set or prints a default error
 * message and aborts.
 */
#define MEM_IFNULL(x) ((ret != NULL)?0:(((mem_outofmem)?mem_outofmem():0), ONS_ABORT("Memory allocation failed, out of memory!")))

/* Allocates a memory block of \size bytes and returns a pointer
 * to it. Calls the global out-of-mem handler if the allocation fails.
 * If \size equals zero the behaviour is undefined.
 * The returned value must be freed with mem_free() or free().
 */
static inline void *mem_malloc(size_t size) {
    void *ret;

    ONS_ASSERT(size > 0);

    ret = malloc(size);
    MEM_IFNULL(ret);
    return ret;
}

/* Same as mem_malloc() but initializes the allocated memory to 0. */
static inline void *mem_zmalloc(size_t size) {
    void *ret;

    ret = mem_malloc(size);
    memset(ret, 0, size);

    return ret;
}

/* This function resizes the memory block where \mem points to to be \size
 * bytes big and returns a pointer to the new memory block.
 * If the old size was smaller than \size the old data is preserved and
 * still available in the new block but probably at another position.
 * If the old size was bigger than \size, the first \size bytes of the old
 * data are preserved.
 * Calls the global out-of-mem handler if the allocation fails.
 * If \size or \mem equals zero the behaviour is undefined.
 * The returned value must be freed with mem_free() or free().
 */
static inline void *mem_realloc(void *mem, size_t size) {
    void *ret;

    ONS_ASSERT(mem != NULL);
    ONS_ASSERT(size > 0);

    ret = realloc(mem, size);
    MEM_IFNULL(ret);
    return ret;
}

/* Duplicates the memory \mem of size \len.
 * The returned value must be freed with mem_free() or free().
 * Calls the global out-of-mem handler if the allocation fails.
 * If \size or \mem equals zero the behaviour is undefined.
 */
static inline void *mem_dup(const void *mem, size_t size) {
    void *ret;

    ONS_ASSERT(mem != NULL);
    ONS_ASSERT(size > 0);

    ret = mem_malloc(size);
    memcpy(ret, mem, size);
    return ret;
}

/* Duplicates the string \str. \str must be terminated with \0.
 * The returned value must be freed with mem_free() or free().
 * Calls the global out-of-mem handler if the allocation fails.
 * If \str equals zero the behaviour is undefined.
 */
static inline char *mem_strdup(const char *str) {
    char *ret;

    ONS_ASSERT(str != NULL);

    ret = strdup(str);
    MEM_IFNULL(ret);
    return ret;
}

/* Frees memory blocks.
 * If \mem is NULL, nothing is done.
 * \mem should be the pointer returned by the allocation functions.
 */
static inline void mem_free(void *mem) {
    if(mem) free(mem);
}


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_alloc_h */

