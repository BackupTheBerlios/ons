/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 15. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 13. May 2009
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


/* API: Machine
 * We compile the constants out of machine.h defined ONS_CONF_* constants. The
 * machine.h header is already included in ons.h.
 */
#define ONS_MACHINE
#ifdef ONS_CONF_LITTLE_ENDIAN
    #define ONS_MACHINE_LITTLEENDIAN
#elif defined(ONS_CONF_BIGENDIAN)
    #define ONS_MACHINE_BIGENDIAN
#else
    #error "Your system was not detected as little- or bigendian system."
#endif

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #define ONS_MACHINE_PACK
#elif defined(ONS_CONF_HAVE_ATTR_PACKED)
    #define ONS_MACHINE_ATTR
#else
    #error "Your system supports neither '#pragma pack' nor '__attribute__((__packed__))'."
#endif

#ifndef ONS_CONF_NO_IPV6
    #define ONS_MACHINE_IPV6
#endif

#ifdef ONS_CONF_DEBUG
    #define ONS_MACHINE_DEBUG 5
#endif


/* API: System
 * This API is required and must be supported on all systems.
 */
#define ONS_SYSTEM
#include <stdint.h>


/* API: String
 * This API is available on linux systems. The previously defined API "Machine"
 * also makes "strnlen" available when we include "string.h".
 */
#define ONS_STRING
#define ONS_STRING_STRNLEN
#include <string.h>


/* API: Thread
 * PThread should be available on linux systems.
 */
#define ONS_THREAD
#define ONS_THREAD_PTHREAD


/* API: Time
 * This is provided through glibc.
 */
#define ONS_TIME
#define ONS_TIME_GTOD


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_linux_h */

