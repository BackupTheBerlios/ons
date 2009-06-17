/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 25. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* MacOS configuration.
 *
 * This configuration runs on several Apple Macintosh machines.
 * This comment lists all platforms which are served by this header.
 *
 * Definitions file for all MacOS platforms. This includes:
 *  - Mac OS 8
 *  - Mac OS 9
 *  - Mac OS X
 * On all of the platforms supported by these systems. These are:
 *  - x86
 *  - x86_64
 *  - ppc
 *  - ppc64
 *  - arm
 * This file is not supposed to be included on any other platform.
 */

/* Check endianness.
 * "sys/param.h" is available on all MAC systems so we use it to get
 * the required symbols.
 */
#include <sys/param.h>
#if defined(__LITTLE_ENDIAN__)
    #define ONS_ARCH_LITTLEENDIAN
#elif defined(__BIG_ENDIAN__)
    #define ONS_ARCH_BIGENDIAN
#else
    #error "macosx.machine.h: Could not detect endiannes; ist this really a Mac OS system?"
#endif

/* There are plenty of thread libraries inside the many Mac APIs, however, they all
 * depend on pthread, thus, we simply use our pthread backend which should fit best.
 */
#define ONS_THREAD_PTHREAD

/* GetTimeOfDay() is available on Mac OS X
 * as on any other good Unix system.
 */
#define ONS_TIME_GTOD

/* The Berkeley Socket API is available.
 * However, EXTSOCK is not available here.
 */
#define ONS_SOCKET_BERKELEY_HEADERS
#define ONS_SOCKET_IOCTL
#define ONS_SOCKET_FCNTL
#define ONS_SOCKET_ALEN
