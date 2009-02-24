/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 19. April 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. January 2009
 */

/* Main and public header of the SAW library.
 * You have to include this header if you want to use the library. This
 * header includes ALL other headers in the right order, thus, you can
 * use all features.
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_saw_h
#define ONS_INCLUDED_saw_saw_h
ONS_EXTERN_C_BEGIN


/* Include all headers in right order. */
#include <saw/addr4.h>      /* IPv4 address manipulating header */
#include <saw/addr6.h>      /* IPv6 address manipulating header */
#include <saw/addrx.h>      /* wrapper for IPv4 and IPv6 addresses */
#include <saw/addrlo.h>     /* file address manipulating header */
#include <saw/addr.h>       /* address superset header */


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_saw_h */

