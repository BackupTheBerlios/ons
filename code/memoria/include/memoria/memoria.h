/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* Main and public header of memoria.
 * This header makes all memoria functionality with all dependencies
 * visible to the user.
 */


#include <sundry/sundry.h>

#ifndef MEMORIA_INCLUDED_memoria_memoria_h
#define MEMORIA_INCLUDED_memoria_memoria_h
SUNDRY_EXTERN_C_BEGIN


#include <stdint.h>

/* Classic strnlen() (Not part of C89/C99!)
 * Find the length of \string, but scan at most \len characters. If no
 * '\0' terminator is found in that many characters, return \len.
 * If '\0' is found the length is returned without the zero character.
 * If \string is NULL, 0 is returned.
 */
static size_t mem_strnlen(const char *string, size_t len) {
    unsigned int i;

    if(string == NULL) return 0;
    for(i = 0; i < len; ++i) if(string[i] == '\0') return i;
    return len;
}


/* Hashes arbitrary values.
 * Takes as argument a pointer to an array \str of \len bytes and returns
 * a hashed value for this array. The algorithm is really fast, however,
 * it is not intended for usage in cryptographic functions.
 * The hash function is created to change the hash strongly if even only one bit
 * changes in the whole array.
 *
 * If you don't know how to create an hashtable out of a this hash function, please
 * see the documentation of Memoria which shows how to create a hashtable.
 * If you don't want to create a hashtable yourself, use any API provided by Memoria
 * that uses this hash function.
 */
typedef uint32_t mem_hash_t;
extern uint32_t mem_hash(const char *str, size_t len);


/* Compare two elements.
 * The type of \comparison and \original depend on the context where this callback
 * is called. However, they are almost always of the same type.
 * The callback should return:
 *   < 0 if /comparison is smaller than /original
 *   = 0 if both are the same
 *   > 0 if /comparison is greater than /original
 */
typedef signed int (*mem_match_t)(const void *comparison, const void *original);


/* Bit manipulation.
 * These macros manipulate the single bits of a value.
 * MEM_LROT(bit, val, k) rotates the \bit bit value \val for \k bytes to the left.
 * MEM_RROT(bit, val, k) rotates the \bit bit value \val for \k bytes to the right.
 * - MEM_LROT(32, 0x1, 4) would rotate the 32bit value 0x1 for 4 bits to the left and, hence, return 0x10.
 * MEM_BIT(x) returns a value with the \x'th bit set. (0 is the first bit)
 * MEM_BITS(x) returns a \x bit value with all bits set. (even the last bit of 64bit is supported)
 */
#define MEM_LROT(bit, val, k) (( (val) << (k) ) | ( (val) >> ((bit) - (k)) ))
#define MEM_RROT(bit, val, k) (( (val) >> (k) ) | ( (val) << ((bit) - (k)) ))
#define MEM_BIT(x) (0x1ULL << (x))
#define MEM_BITS(x) ((MEM_BIT((x) - 1) - 1) | MEM_BIT((x) - 1))


/* Mask manipulation macro.
 * These macros help you moving masks to other positions.
 *
 * MEM_MASK_HIGH(bits, mask) Returns a value of size \bits with the most upper bits set.
 *                           \mask must be a power of 2.
 * \bits cannot be bigger than 64.
 * Example:
 *   MEM_MASK_HIGH(16, 8) represents two values (in binary):
 *     \bits: 0000 0000 0000 0000
 *     \mask: 0000 0000 0000 1000
 *   Now this macro converts the mask into:
 *     \mask: 0000 0000 0000 0111
 *   that is, it sets all bits below the mask. Remember, \mask has to be a power of 2!
 *   At last step, this macro moves this mask to the most upper position:
 *     \mask: 1110 0000 0000 0000
 *   and returns this value.
 *
 * MEM_MASK_LOW(bits, mask) Same as MEM_MASK_HIGH but sets the lowest bits.
 *
 * MEM_MASK_MOV(bits, mask, value) Moves the mask to the left. Returns 0 if it cannot be moved.
 * Example:
 *   MEM_MASK_MOV(16, 8, 7):
 *     \bits: 0000 0000 0000 0000
 *     \mask: 0000 0000 0000 1000
 *     \value: 0000 0000 0000 0111
 *   returns:
 *     \value: 0000 0000 0011 1000
 *   If called with:
 *     \value: 0011 1000 0000 0000
 *   then it would return 0, because there is not enough space to move the mask to.
 */
#define MEM_MASK_HIGH(bits, mask) (~(MEM_BITS(bits) / (mask)))
#define MEM_MASK_LOW(bits, mask) ((mask) - 1)
#define MEM_MASK_MOV(bits, mask, value) (((value) & MEM_MASK_HIGH((bits), (mask)))?0:((value) * (mask)))


/* Large flagset manipulation.
 * This allows to create a flagset of unlimited size. That is, you can store as many flags as
 * you want in this flagset, however, the amount of flags has to be defined at compile time.
 *
 * Usage example:
 *   enum {
 *     FLAG1,
 *     FLAG2,
 *     ...
 *     FLAG100,
 *     FLAG_NUM
 *   };
 *
 *   Important: "FLAG_NUM" is no flag and cannot be stored in the bitset. It just contains the number
 *              of flags.
 *
 *   MEM_DECLARE_FLAGSET(example_bitset, FLAG_NUM);
 *
 *   example_bitset bitset;
 *   MEM_SETFLAG(&bitset, FLAG2);
 *   if(MEM_HASFLAG(&bitset, FLAG2)) ...
 *   MEM_CLRFLAG(&bitset, FLAG2);
 */
typedef unsigned long mem_flagpage_t;
#define MEM_FLAGPAGE_BITS (8 * sizeof(mem_flagpage_t))
#define MEM_FLAGSET_INDEX(flag) ((flag) / MEM_FLAGPAGE_BITS)
#define MEM_FLAGSET_MASK(flag) (1UL << ((flag) % MEM_FLAGPAGE_BITS))
#define MEM_DECLARE_FLAGSET(name, max) \
    typedef struct name { \
        mem_flagpage_t bits[((max + MEM_FLAGPAGE_BITS - 1) / MEM_FLAGPAGE_BITS)]; \
    } name;
#define MEM_HASFLAG(set, flag) ((set)->bits[MEM_FLAGSET_INDEX(flag)] & MEM_FLAGSET_MASK(flag))
#define MEM_SETFLAG(set, flag) ((set)->bits[MEM_FLAGSET_INDEX(flag)] |= MEM_FLAGSET_MASK(flag))
#define MEM_CLRFLAG(set, flag) ((set)->bits[MEM_FLAGSET_INDEX(flag)] &= ~MEM_FLAGSET_MASK(flag))


#include <memoria/alloc.h>
#include <memoria/array.h>
#include <memoria/list.h>


SUNDRY_EXTERN_C_END
#endif /* MEMORIA_INCLUDED_memoria_memoria_h */

