/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 13. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. June 2009
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
    #include <unistd.h>
#endif

#ifdef ONS_TIME_WIN
    #include <windows.h>
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


void sundry_timediff(sundry_time_t *buf, sundry_time_t *ctime2, sundry_time_t *ptime) {
    sundry_time_t *big, *small;

    SUNDRY_ASSERT(buf && ctime2 && ptime);

    /* Check which one is bigger. */
    if(ctime2->secs > ptime->secs) {
        big = ctime2;
        small = ptime;
    }
    else if(ctime2->secs < ptime->secs) {
        big = ptime;
        small = ctime2;
    }
    else if(ctime2->usecs > ptime->usecs) {
        big = ctime2;
        small = ptime;
    }
    else if(ctime2->usecs < ptime->usecs) {
        big = ptime;
        small = ctime2;
    }
    else {
        memset(buf, 0, sizeof(sundry_time_t));
        return;
    }

    buf->secs = big->secs - small->secs;
    if(small->usecs > big->usecs) {
        --buf->secs;
        buf->usecs = 1000000UL + big->usecs - small->usecs;
    }
    else buf->usecs = big->usecs - small->usecs;
}


void sundry_timesum(sundry_time_t *buf, sundry_time_t *ctime2, sundry_time_t *ptime) {
    SUNDRY_ASSERT(buf && ctime2 && ptime);

    buf->secs = ctime2->secs + ptime->secs;
    buf->usecs = ctime2->usecs + ptime->usecs;

    /* Check for microseconds overflow. */
    if(buf->usecs >= 1000000UL) {
        buf->usecs -= 1000000UL;
        ++buf->secs;
    }
}


void sundry_sleep(sundry_time_t *ptime) {
#ifdef ONS_TIME_WIN
    sleep(ptime->secs * 1000UL + ptime->usecs / 1000);
#elif defined(ONS_TIME_GTOD)
    usleep(ptime->secs * 1000000UL + ptime->usecs);
#endif
}

