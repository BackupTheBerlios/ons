/*
 * (COPYRIGHT) Copyright (C) 2008, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. January 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 5. January 2009
 */

/* Miscellaneous protofcgi definitions and checks.
 * Tests the system for compatibility with this library and defines some
 * necessary macros.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_protofcgi_misc_h
#define ONS_INCLUDED_protofcgi_misc_h
ONS_EXTERN_C_BEGIN


/* We include memoria, saw and fip here.
 * They are used in almost every header/source file.
 */
#include <memoria/memoria.h>
#include <saw/saw.h>
#include <fip/fip.h>

/* Key<->Value pairs.
 * This type is used for key value pairs.
 * "c" stands for constant and "s" for "size-limited".
 * \klen and \vlen contain the size of the string without the
 * terminating \0 character which must be written in the string.
 * The "spair" and "cspair" allow \0 to be at every position in
 * the string. In the other structs \0 terminates the string.
 */
typedef struct cgi_pair_t {
    char *key;
    char *value;
} cgi_pair_t;
typedef struct cgi_spair_t {
    size_t klen;
    size_t vlen;
    char *key;
    char *value;
} cgi_pair_t;
typedef struct cgi_cpair_t {
    const char *key;
    const char *value;
} cgi_cpair_t;
typedef struct cgi_cspair_t {
    size_t klen;
    size_t vlen;
    const char *key;
    const char *value;
} cgi_cpair_t;


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_protofcgi_misc_h */

