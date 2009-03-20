/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 18. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 20. March 2009
 */

/* Main and public header of memoria.
 * This header makes all memoria functionality with all dependencies
 * visible to the user.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_memoria_memoria_h
#define ONS_INCLUDED_memoria_memoria_h
ONS_EXTERN_C_BEGIN


/* Classic strnlen() (Not part of C89/C99!)
 * Find the length of \string, but scan at most \len characters. If no
 * '\0' terminator is found in that many characters, return \len.
 * If '\0' is found the length is returned without the zero character.
 * If \string is NULL, 0 is returned.
 */
#ifdef ONS_STRING_STRNLEN
    #include <string.h>
    #define mem_strnlen(x, y) strnlen((x), (y))
#else
    static inline size_t mem_strnlen(const char *string, size_t len) {
        unsigned int i;

        if(string == NULL) return 0;
        for(i = 0; i < len; ++i) if(string[i] == '\0') return i;
        return len;
    }
#endif


/* Bit manipulation.
 * These macros manipulate the single bits of a value.
 * MEM_LROT(bit, val, k) rotates the \bit bit value \val for \k bytes to the left.
 * MEM_RROT(bit, val, k) rotates the \bit bit value \val for \k bytes to the right.
 * - MEM_LROT(32, 0x1, 4) would rotate the 32bit value 0x1 for 4 bits to the left and, hence, return 0x10.
 * MEM_BIT(x) returns a value with the \x'th bit set.
 */
#define MEM_LROT(bit, val, k) (( (val) << (k) ) | ( (val) >> ((bit) - (k)) ))
#define MEM_RROT(bit, val, k) (( (val) >> (k) ) | ( (val) << ((bit) - (k)) ))
#define MEM_BIT(x) (0x1 << (x))


#include <memoria/alloc.h>
#include <memoria/array.h>
#include <memoria/list.h>


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_memoria_h */

