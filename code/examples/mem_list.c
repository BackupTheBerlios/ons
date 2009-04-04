/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 3. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 4. April 2009
 */

/* This example shows how to use the linked list interfaces
 * of libmemoria.
 */


#include "include.h"
#include <memoria/memoria.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/* How this testsuite works.
 * This testsuite is created to test the list backend for bugs. It consists
 * of a function which performs randomly an action on the list. You should
 * call this function in a loop and check the memory consumption in a process
 * manager in the meantime.
 */


#define ROUND1 1000000ULL
#define ROUND2 10000000000ULL
MEM_ARRAY_DEFINE(tmplist, mem_node_t*, 100, 0, 1, NULL);
tmplist glist;


uint64_t count = 0;
uint64_t dupl = 0;
static inline void list_insert(mem_list_t *list) {
    mem_node_t *node;
    uint32_t num, key[100];

    node = mem_node_new(list->type);

    new_rand:

    num = mem_rand(NULL);
    key[99] = num;
    mem_node_setkey(node, key, sizeof(key));
    if(mem_list_insert(list, node) != node) {
        ++dupl;
        goto new_rand;
    }

    glist.list[count] = node;
    ++count;
}


static inline void list_shuffle(mem_list_t *list) {
    uint32_t num, num2;
    mem_node_t *tmp;

    num = mem_rand(NULL) % ROUND1;
    num2 = mem_rand(NULL) % ROUND1;

    tmp = glist.list[num];
    glist.list[num] = glist.list[num2];
    glist.list[num2] = tmp;
}


static inline void list_find(mem_list_t *list) {
    mem_node_t *node;

/* This could be taken instead of the "560"-random below. However,
 * MODULO "%" is REALLY slow and would make this example 100 times slower.
 */
/*
    uint32_t num;
    num = mem_rand(NULL) % ROUND1;
    node = glist.list[num];
*/

    /* take randomly one node. \560 is totally randomly choosen */
    node = glist.list[560];
    if(list->type == MEM_TABLE)
        ONS_ASSERT(mem_list_find(list, &node->hash, 0));
    else
        ONS_ASSERT(mem_list_find(list, node->key, node->len));
}


static inline void list_remove(mem_list_t *list) {
    mem_node_t *node;

    node = glist.list[--count];

    mem_list_remove(list, node);
    mem_node_free(node);
}


static inline void list_test(mem_list_t *list) {
    unsigned long long l;
    time_t begin, end;

    printf("Inserting %llu elements randomly:\n", ROUND1);
    begin = time(0);
    for(l = 0; l < ROUND1; ++l) {
        list_insert(list);
    }
    end = time(0);
    printf("  %li seconds (%llu duplicates)\n", end - begin, dupl);
    dupl = 0;

    printf("Shuffling the elements...\n");
    for(l = 0; l < ROUND1; ++l) {
        list_shuffle(list);
    }

    printf("Finding %llu elements randomly:\n", ROUND2);
    begin = time(0);
    for(l = 0; l < ROUND2; ++l) {
        list_find(list);
    }
    end = time(0);
    printf("  %li seconds\n", end - begin);

    printf("Removing %llu elements randomly:\n", ROUND1);
    begin = time(0);
    for(l = 0; l < ROUND1; ++l) {
        list_remove(list);
    }
    end = time(0);
    printf("  %li seconds\n", end - begin);
}


static inline void test_list(int type) {
    mem_list_t *list;

    tmplist_init(&glist);
    tmplist_resize(&glist, ROUND1);
    list = mem_list_new(type);
    list_test(list);
    mem_list_free(list);
    tmplist_clear(&glist);
}

signed int main(signed int argc, char *argv[]) {
    mem_seed(NULL, time(NULL));
    printf("Starting TEST-Suite for Red-Black Tree:\n");
    test_list(MEM_RBTREE);
    printf("\nStarting TEST-Suite for Splay Tree:\n");
    test_list(MEM_SPLAY);
    printf("\nStarting TEST-Suite for Hash Table:\n");
    test_list(MEM_TABLE);
    printf("\nStarting TEST-Suite for Greedy Hash Table:\n");
    test_list(MEM_TABLE_GREEDY);
    printf("\nStarting TEST-Suite for Static Hash Table:\n");
    test_list(MEM_TABLE_STATIC);
    return EXIT_SUCCESS;
}

