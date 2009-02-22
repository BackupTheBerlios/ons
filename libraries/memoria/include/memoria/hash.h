/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 22. February 2009
 */

/* Hash Table implementation.
 * This interface almost implements an hash table. An hash table is based
 * on a fast algorithm which eases the task to compare keys. This makes
 * especially searching for long strings very fast. However, hash tables
 * can be implemented differently and we don't want to decide the backend
 * of the hash table for you, thus, you can use our hashing algorithm to
 * use it with your own backend (possible array.h or llist.h from this lib).
 *
 * This file contains a short example on how to implement array.h in your
 * hash table. See the comment below.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_hash_h
#define ONS_INCLUDED_memoria_hash_h
ONS_EXTERN_C_BEGIN


/* Hashes arbitrary values.
 * Takes as argument a pointer to an array \str of \len bytes and returns
 * a hashed value for this array. The algorithm is really fast, however,
 * it is not intended for usage in cryptographic functions.
 * The hash function is created to change the hash strongly if even only one bit
 * changes in the whole array.
 */
extern mem_hash_t mem_hash(const char *str, size_t len);

/* Example implementation of an hash table with array.h and mem_hash().
 * mem_hash_t is guaranteed to be 32byte wide, hence, we can use it to
 * get four 256bit values. We make the hash table 256 byte big and search
 * up to four times for a key.
 * The hash table would be:
 * > MEM_ARRAY_DECLARE(test_hash_t, test_type_t, 1);
 * > test_hash_t hash_table[256];
 * If you want to insert a new value, you just create the hash:
 * > my_hash = mem_hash(key, sizeof(key));
 * Now we got one hash value of four 256bit hashes. We take the first 256bit value
 * with my_hash & 0xff. We check whether this array is empty. If it is, we insert
 * the value here.
 * > test_hash_t_empty(&hash_table[my_hash & 0xff])
 * If it is not empty, we check the next array:
 * > test_hash_t_empty(&hash_table[my_hash & 0xff00])
 * If this is not empty, too, we check also the third and fourth array:
 * > test_hash_t_empty(&hash_table[my_hash & 0xff0000])
 * > test_hash_t_empty(&hash_table[my_hash & 0xff000000])
 * If even the last array is not empty, we insert it in the last array:
 * > test_type_t *p = test_hash_t_push(&hash_table[my_hash & 0xff000000]);
 * Now we can fill *p with data.
 * If we want to find a value, we just do the same task but instead of checking
 * whether the arrays are empty, we check whether the FIRST element has the same
 * real key (not hash) which we are searching for. If even the fourth array does
 * not match, we search through the whole fourth array.
 * If you additionally want to link all elements in all arrays, you have to create
 * a linked list in the test_type_t and add the elements to this list, too.
 *
 * If your hash table gets bigger you could even increase the hash table size from
 * 256 to another arbitrary value, but I recommend using powers of two. With up to
 * 65536 it is possible to get 2 hash values out of an mem_hash_t. But 65536 would
 * even be too big for the biggest hash tables.
 *
 * You could also replace the array with a linked list to be more flexible with many
 * elements in a little hash table. However, hash tables with more than 5 elements
 * in one bucket (bucket == one element in the hash-table array) are not that fast
 * anymore and I recommend using B-trees or other similar arlgorithms like red-black-trees,
 * B+-trees, B*-trees or even something totally different.
 */


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_hash_h */

