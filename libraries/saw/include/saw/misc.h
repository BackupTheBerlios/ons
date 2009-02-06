/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 19. April 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 17. January 2009
 */

/* Miscellaneous SAW definitions and checks.
 * Tests the system for compatibility with this library and defines some
 * necessary macros.  Should be the first file included in all other headers
 * of the SAW library.
 * Most of the checks are outsourced in autoconf, this file parses the
 * output of autoconf (ons/setup.h) and sets the correct values.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_saw_misc_h
#define ONS_INCLUDED_saw_misc_h
ONS_EXTERN_C_BEGIN


#include <limits.h>
#include <stdlib.h>

/* include standard headers */
#ifdef ONS_CONF_HAVE_SYS_TYPES_H
    #include <sys/types.h>
#endif

typedef uint16_t saw_port_t;


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_misc_h */

