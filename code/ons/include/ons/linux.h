/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 15. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* LINUX specific API description
 * This file defines the API described in ons.h for linux systems. This file
 * only supports linux-2.6. Maybe other linux kernel versions are supported
 * in future, but currently, there is no support for them.
 *
 * David Herrmann -- 2009/03/15
 */


#ifndef ONS_INCLUDED_ons_linux_h
#define ONS_INCLUDED_ons_linux_h
#ifdef __cplusplus
extern 'C' {
#endif


/* API: System
 * This API is required and must be supported on all systems.
 */
#define ONS_SYSTEM


/* API: String
 * This API is available on linux systems. The previously defined API "Machine"
 * also makes "strnlen" available when we include "string.h".
 */
#define ONS_STRING
#define ONS_STRING_STRNLEN
#include <string.h>


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_linux_h */

