/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Manual Heap Management.
 *  This interface allows to create an own heap. It allocates huge memory blocks and
 *  maps the requestes values into the memory. The advantage is, that every memory
 *  allocated through this interface is given back to the system after it is freed.
 *  However, this makes it more than twice as slow as using "malloc" all the time.
 *  If the mmap() system is not available this heap uses the normal malloc() routine.
 * Implementation Details:
 *  A mem_heap_t controlls several "mem_block_t"s. Each "mem_block_t" has a fixed amount
 *  of childs where each child represents one value. The memory of a child is preceded
 *  by a "mem_node_t" which helps us finding free blocks much faster.
 */

/* Benchmarks on my system.
 * I compared this heap with the normal "malloc" implementation.
 *  nodes_per_block: 10
 *  nodes allocated: 100
 * I used node_size with 10, 100, 1000, 10000 and all time the heap
 * was twice as slow up to 2/3 as slow as malloc().
 * Using node_size with >100KB makes this heap really slow. It seems that
 * mmap(MAP_ANONYMOUS) is not this good at big sizes.
 *  nodes_per_block: 100
 *  nodes allocated: 100
 * If node_size <100KB the heap is as fast as malloc() (or almost nearly as fast).
 * However, who uses such values?
 *  nodes_per_block: 10
 *  nodes allocated: 10000
 * Also twice as slow as malloc() (or little bit slower) with node_size <100KB.
 *
 * Conclusion: Though this system is slower than malloc() there are reasons why to use it.
 * A short introduction: The virtual memory of every process is limited to your memory size.
 * If the kernel makes a process active, it maps all accesses from the process to it's virtual
 * memory to the real memory. It manages memory-maps which help it to do this task.
 * Everytime when you invoke malloc() the memory you requested is added to the pool of malloc().
 * If you free your memory with free() it's not removed from the pool of malloc(). Thus, the
 * kernel memory-maps which help to map the virtual memory to the real memory won't get smaller.
 * However, the real memory does not keep the values you free()ed endlessly, which is a widespread
 * misbelief.
 *
 * If you use mmap() the memory you free() is removed from the memory-maps. Thus, if the kernel
 * switches the active process and needs to load the memory-maps, it has to load a much smaller
 * map and will succeed faster.
 * This shows that the usage of this heap system is only beneficial if you need to allocate a
 * huge amount of memory for a short time in a process which will run for long time.
 *
 * Perhaps, you may say that malloc() is still faster. And yes, I agree. I implemented this system
 * because great software like the hybrid-ircd used it and I wanted to test whether this is really
 * faster. However, I never tested it in a real application with the requirements mentioned above.
 * If you developed such a application and benchmarked it, I would be glad to hear your results.
 * Furthermore, this shows how fast modern malloc() implementations really are and that there is
 * no need to write own heap mechanisms.
 */

#include "memoria/memoria.h"

mem_heap_t *mem_heap_create(size_t node_size, unsigned long nodes_per_block) {
    mem_heap_t *heap;
    unsigned int i;
    void *mem;
    mem_heap_node_t *node;

    if(node_size == 0 || nodes_per_block == 0) return NULL;
    if(!(heap = mem_malloc(sizeof(mem_heap_t) + sizeof(mem_heap_block_t)))) return NULL;

    /* initialize heap */
    heap->node_size = node_size;
    heap->nodes_per_block = nodes_per_block;
    heap->block_count = 1;
    heap->free = nodes_per_block;
    heap->blocks = sizeof(mem_heap_t) + (void*)heap;

    /* initialize block */
    heap->blocks->next = heap->blocks;
    heap->blocks->mem = mem_block_alloc(nodes_per_block * (node_size + sizeof(mem_heap_node_t)));
    if(!heap->blocks->mem) {
        mem_free(heap);
        return NULL;
    }
    heap->blocks->free = nodes_per_block;
    heap->blocks->nodes = heap->blocks->mem;

    /* initialize nodes */
    mem = heap->blocks->mem;
    for(i = 0; i < nodes_per_block; ++i) {
        node = mem;
        node->block = heap->blocks;
        if(i) ((mem_heap_node_t*)(mem - sizeof(mem_heap_node_t) - node_size))->next = node;
        mem += sizeof(mem_heap_node_t) + node_size;
    }
    node->next = heap->blocks->nodes;

    return heap;
}

void mem_heap_destroy(mem_heap_t *heap) {
    mem_heap_block_t *iter;

    iter = heap->blocks->next;
    while(iter != heap->blocks) {
        iter = heap->blocks->next;
        heap->blocks->next = heap->blocks->next->next;
        mem_block_free(iter->mem, heap->nodes_per_block * (heap->node_size + sizeof(mem_heap_node_t)));
        free(iter);
    }
    mem_block_free(heap->blocks->mem, heap->nodes_per_block * (heap->node_size + sizeof(mem_heap_node_t)));
    /* first block is allocated with the heap */
    mem_free(heap);
}

void *mem_heap_alloc(mem_heap_t *heap) {
    mem_heap_block_t *iter;
    mem_heap_node_t *node;
    unsigned int i;
    void *mem;

    /* Either we have free nodes left or we need to allocate a new block. */
    if(heap->free > 0) {
        /* Find a block with free nodes. Start not at first block to prevent searching
         * the root block everytime for free nodes. Furthermore, new blocks are inserted
         * at the second position.
         */
        iter = heap->blocks->next;
        do {
            if(iter->free == 0) iter = iter->next;
            else if(iter->free == 1) {
                node = iter->nodes;
                iter->nodes = NULL;
                --heap->free;
                --iter->free;
                return (sizeof(mem_heap_node_t) + (void*)node);
            }
            else {
                node = iter->nodes->next;
                iter->nodes->next = iter->nodes->next->next;
                --heap->free;
                --iter->free;
                return (sizeof(mem_heap_node_t) + (void*)node);
            }
        } while(iter != heap->blocks->next);

        /* the execution should never get here */
        return NULL;
    }
    else {
        /* create a new block */
        if(!(iter = mem_malloc(sizeof(mem_heap_block_t)))) return NULL;

        /* initialize block */
        iter->mem = mem_block_alloc(heap->nodes_per_block * (heap->node_size + sizeof(mem_heap_node_t)));
        if(!iter->mem) {
            mem_free(iter);
            return NULL;
        }

        /* insert it behind the root node */
        iter->next = heap->blocks->next;
        heap->blocks->next = iter;

        iter->free = heap->nodes_per_block - 1;
        heap->free = heap->nodes_per_block - 1;
        ++heap->block_count;
        iter->nodes = iter->mem;

        /* initialize nodes except the last one */
        mem = iter->mem;
        if(heap->nodes_per_block > 1) {
            for(i = 1; i < heap->nodes_per_block; ++i) {
                node = mem;
                node->block = iter;
                if(i > 1) ((mem_heap_node_t*)(mem - sizeof(mem_heap_node_t) - heap->node_size))->next = node;
                mem += sizeof(mem_heap_node_t) + heap->node_size;
            }
            node->next = iter->nodes;
        }
        else iter->nodes = NULL;

        /* initialize last node */
        node = mem;
        node->block = iter;

        /* return last node */
        return mem + sizeof(mem_heap_node_t);
    }
}

void mem_heap_free(mem_heap_t *heap, void *mem) {
    mem_heap_node_t *node;
    mem_heap_block_t *block;

    node = mem - sizeof(mem_heap_node_t);
    if(node->block != heap->blocks && node->block->free == heap->nodes_per_block - 1) {
        /* If a block is unused and not the root block, delete it. */
        block = heap->blocks;
        do {
            if(block->next == node->block) goto found;
            block = block->next;
        } while(block != heap->blocks);

        found:
        block->next = block->next->next;
        heap->free -= heap->nodes_per_block - 1;
        --heap->block_count;

        /* node gets deleted, thus, save block pointer */
        block = node->block;

        /* delete block */
        mem_block_free(block->mem, heap->nodes_per_block * (heap->node_size + sizeof(mem_heap_node_t)));
        mem_free(block);
    }
    else if(!node->block->nodes) {
        node->next = node;
        node->block->nodes = node;
        memset(mem, 0, heap->node_size);
        ++heap->free;
        ++node->block->free;
    }
    else {
        node->next = node->block->nodes->next;
        node->block->nodes->next = node;
        memset(mem, 0, heap->node_size);
        ++heap->free;
        ++node->block->free;
    }
}

