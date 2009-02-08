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

/* Non-standard string manipulating functions. Most
 * of them are available on many platforms and this file
 * provides a wrapper and a ISO-C conform (slow) implementation.
 */

#include <memoria/misc.h>

#ifndef ONS_INCLUDED_memoria_string_h
#define ONS_INCLUDED_memoria_string_h
ONS_EXTERN_C_BEGIN


#include <stdlib.h>
#include <string.h>

/* Classic strnlen() (Not part of C89/C99!)
 * Find the length of $string, but scan at most $len characters.  If no
 * '\0' terminator is found in that many characters, return $len.
 * If '\0' is found the length is returned without the zero character.
 */
#ifdef ONS_CONF_HAVE_STRNLEN
    #define mem_strnlen(x, y) strnlen((x), (y))
#else
    static inline size_t mem_strnlen(const char *string, size_t len) {
        unsigned int i;

        assert(string != NULL);

        for(i = 0; i < len; ++i) if(string[i] == '\0') return i;
        return len;
    }
#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_string_h */

