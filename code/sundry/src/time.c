/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 13. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* Time controlling API.
 * This API is similar to <time.h> but is much more precise.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "sundry/time.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>

#ifdef ONS_TIME_GTOD
    #include <sys/time.h>
#endif


void sundry_time(sundry_time_t *buf) {
#ifdef ONS_TIME_GTOD
    struct timeval tv;
#elif defined(ONS_TIME_WIN)
    /* Did I already mention that this really sucks? */
    FILETIME o_filetime;
    uint64_t o_utime = 0;
#endif

    SUNDRY_ASSERT(buf != NULL);

#ifdef ONS_TIME_GTOD
    gettimeofday(&tv, NULL);
    buf->secs = tv.tv_sec;
    buf->usecs = tv.tv_usec;
#elif defined(ONS_TIME_WIN)
    /* Get the system's time. */
    GetSystemTimeAsFileTime(&o_filetime);

    /* Assemble the real time with the data we got. */
    o_utime |= o_filetime.dwHighDateTime;
    o_utime <<= 32;
    o_utime |= o_filetime.dwLowDateTime;
 
    /* Converting file time to unix epoch.
     * Wooouuh, this is so slow that it almost makes the whole
     * microseconds issue totally useless.
     * Perhaps, due to this Windows does only return milliseconds
     * and not microseconds. Haha.
     */
    o_utime /= 10;
    o_utime -= 11644473600000000ULL;
    buf->secs = (o_utime / 1000000ULL);
    buf->usecs = (o_utime % 1000000ULL);
#else
    buf->usecs = 0;
    buf->secs = time(NULL);
#endif
}

