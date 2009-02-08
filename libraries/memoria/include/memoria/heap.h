/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 8. February 2009
 */

/* Manual Heap Management.
 * This interface allows to create an own heap. It allocates huge memory blocks and
 * maps the requestes values into the memory. The advantage is, that every memory
 * allocated through this interface is given back to the system after it is freed.
 * However, this makes it more than twice as slow as using "malloc" all the time.
 * If the mmap() system is not available this heap uses the normal malloc() routine.
 * See the comment at the beginning of heap.c for detailed information and a benchmark.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_heap_h
#define ONS_INCLUDED_memoria_heap_h
ONS_EXTERN_C_BEGIN


#include <memoria/mem.h>

/* forward declaration */
struct mem_heap_node_t;
struct mem_heap_block_t;
struct mem_heap_t;

/* Represents a single node which is is managed by a block.
 * \next is a pointer to the next free node. If this node
 * is used (not free) \next is undefined.
 * \block points to the block holding this node.
 */
typedef struct mem_heap_node_t {
    struct mem_heap_node_t *next;
    struct mem_heap_block_t *block;
} mem_heap_node_t;

/* One block of a heap holding a fixed size of nodes.
 * \next points to the next block in the heap. \mem
 * points to the allocated space where the nodes lay.
 * \free contains the number of free nodes in this block
 * and \nodes points to the first free node in this block.
 * If there is no free node, \nodes is undefined.
 */
typedef struct mem_heap_block_t {
    struct mem_heap_block_t *next;
    void *mem;
    unsigned long free;
    mem_heap_node_t *nodes;
} mem_heap_block_t;

/* Single heap structure.
 * This is the main structure holding an heap. If you created such an
 * object you can allocate memory for elements. \node_size contains
 * the size of a single node which can be stored in this heap.
 * \nodes_per_block contains the number of nodes which can be saved
 * in one single block. \block_count contains the number of currently
 * allocated blocks in this heap. \free contains the number of all
 * currently free nodes in all blocks of this heap. \blocks points
 * to the first block. There is always at least one block.
 */
typedef struct mem_heap_t {
    size_t node_size;
    unsigned long nodes_per_block;
    unsigned long block_count;
    unsigned long free;
    mem_heap_block_t *blocks;
} mem_heap_t;

/* Returns a pointer to a new allocated heap.
 * \node_size must be the size of the element you want to store in this heap.
 * \nodes_per_block is the number of elements you want to save in one block. This
 * is just an optimation value. If you set it to a low number, then the heap must
 * often allocate new blocks. If you set it to a high number, there can be much
 * unused space.
 * \node_size and \nodes_per_block must not be zero.
 */
extern mem_heap_t *mem_heap_create(size_t node_size, unsigned long nodes_per_block);

/* Destroys an heap allocated with mem_heap_create.
 * This frees all allocated memory. You do not need to mem_heap_free() all
 * elements before calling this.
 * After calling this, you cannot use the heap anymore.
 * \heap must point to a valid value.
 */
extern void mem_heap_destroy(mem_heap_t *heap);

/* Returns a pointer to a new allocated element. Returns NULL
 * if the memory allocation failed.
 * Every memory allocated is initialized to 0.
 */
extern void *mem_heap_alloc(mem_heap_t *heap);

/* Frees an element allocated with mem_heap_alloc().
 * The heap keeps track of all allocated elements and deletes them if you
 * destroy the heap.
 */
extern void mem_heap_free(mem_heap_t *heap, void *mem);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_heap_h */

