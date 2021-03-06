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

/* This file contains global declarations for the memoria library.
 * It declares macros used in every other header and source of this library.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_memoria_misc_h
#define ONS_INCLUDED_memoria_misc_h
ONS_EXTERN_C_BEGIN


/* Type of the index which is used to access elements directly. */
typedef size_t mem_index_t;

/* Type used for hashes. */
typedef uint32_t mem_hash_t;

/* Expands to two arguments, the first is the string and the second the
 * length of the string until the first zero character.
 */
#define MEM_STR(str) (str), strlen(str)

/* Expands to two arguments, a pointer to string-literal and the length of the string-literal. */
#define MEM_STRL(str) (str), (sizeof(str) - 1)

/* Expands to two arguments, the string and the length of the string until
 * the first zero character or \len, depending on which occurs first.
 */
#define MEM_STRN(str, len) (str), strnlen((str), (len))

/* Expands to two arguments, a pointer to array and the size of the array. */
#define MEM_LIT(str) (str), sizeof(str)


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_memoria_misc_h */

