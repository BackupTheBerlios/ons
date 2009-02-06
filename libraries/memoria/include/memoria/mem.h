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

/* This file wraps the common memory allocation functions
 * and allows to set an handler to catch out-of-memory
 * exceptions. However, the allocation functions still
 * return NULL if they fail.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_mem_h
#define ONS_INCLUDED_memoria_mem_h
ONS_EXTERN_C_BEGIN


#include <stdlib.h>
#include <string.h>

/* A sample out-of-mem handler.
 * This handler sets an internal state to 1 and returns.
 * If it is called twice and the internal is 1, it terminates
 * the process.
 * This allows to call clean-up routines if the memory allocation
 * failed but if the clean-up routines themself allocate memory
 * the process gets terminated.
 */
extern void mem_outofmem_handler(void);

/* Set this variable to the out of mem handler you want to have.
 * It is initialized with NULL and if it is NULL, no out-of-mem
 * handler is called.
 * Please be aware that the handler can be called parallel in
 * multithreaded programs.
 */
extern void (*mem_outofmem)(void);

/* Allocates a memory block of \size bytes and returns a pointer
 * to it. If the memory allocation fails and either no out-of-mem
 * handler is set or the handler returned, NULL is returned.
 * The returned value can be freed with mem_free().
 */
static inline void *mem_malloc(size_t size) {
    void *ret = malloc(size);

    if(ret == NULL && mem_outofmem) {
        mem_outofmem();
    }
    return ret;
}

/* Same as mem_malloc() but initializes the allocated memory to 0.
 * The returned value can be freed with mem_free().
 */
static inline void *mem_zmalloc(size_t size) {
    void *ret = malloc(size);

    if(ret == NULL && mem_outofmem) {
        mem_outofmem();
    }
    memset(ret, 0, size);
    return ret;
}

/* This function resizes the memory block where \mem points to to be \size
 * bytes big and returns a pointer to the new memory block.
 * If the old size was smaller than \size the old data is preserved and
 * still available in the new block but probably at another position.
 * If the old size was bigger than \size, the first \size bytes of the old
 * data are preserved.
 * mem_realloc() never fails if \size is smaller than the size before.
 * The returned value can be freed with mem_free().
 */
static inline void *mem_realloc(void *mem, size_t size) {
    void *ret = realloc(mem, size);

    if(ret == NULL && mem_outofmem) {
        mem_outofmem();
    }
    return ret;
}

/* Duplicates the memory \mem of size \len.
 * The returned value can be freed with mem_free().
 */
static inline void *mem_dup(const void *mem, size_t len) {
    void *ret = mem_malloc(len);
    if(!ret) return NULL;
    memcpy(ret, mem, len);
    return ret;
}

/* Frees memory blocks.
 * If \mem is NULL, nothing is done.
 * \mem should be the pointer returned by the allocation functions.
 * The memory allocated with the "block" interface (see below) cannot
 * be freed with this function.
 */
static inline void mem_free(void *mem) {
    if(mem) free(mem);
}

/* Same as mem_zmalloc() but tries to allocate the memory
 * through the mmap(MAP_ANONYMOUS) interface. If mmap(MAP_ANONYMOUS)
 * is not available the classic malloc() interface is used.
 *
 * You must free every allocated memory with mem_block_free().
 *
 * Classic memory management with malloc() and free() does not
 * give the memory back to the operating system after freeing it.
 * mmap() gives the memory back. You should use this method only
 * if you plan to allocate a huge memory block for short time and
 * then don't use it, anymore. In all other cases, the traditional
 * malloc() interface is much faster and should be prefered.
 * Additionally, the memory freed with free() is often not available
 * in the real RAM and only kept in the virtual memory mapping
 * tables of the process.
 *
 * I suggest not using this "block" interface in favour of the malloc()
 * interface. Nevertheless if you still want to use it, have a look
 * on the heap implementation in this library.
 */
extern void *mem_block_alloc(size_t size);

/* Frees the memory allocated with mem_block_alloc().
 * You need to pass the same \size parameter as you have passen to
 * mem_block_alloc().
 * \ptr should be the pointer returned by mem_block_alloc().
 */
extern void mem_block_free(void *ptr, size_t size);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_mem_h */

