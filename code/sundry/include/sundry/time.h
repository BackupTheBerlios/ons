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

#include <sundry/sundry.h>

#ifndef SUNDRY_INCLUDED_sundry_time_h
#define SUNDRY_INCLUDED_sundry_time_h
SUNDRY_EXTERN_C_BEGIN


#include <stdint.h>

/* Datatype which contains a positive time specified by seconds
 * and microseconds.
 */
typedef struct sundry_time_t {
    uint64_t secs;
    uint64_t usecs;
} sundry_time_t;

/* Both constants can be passed as additional parameter and specify
 * whether the time in the related \sun_time_t is positive or negative.
 */
#define SUNDRY_TIME_POSITIVE 0
#define SUNDRY_TIME_POS SUN_TIME_POSITIVE
#define SUNDRY_TIME_NEGATIVE 1
#define SUNDRY_TIME_NEG SUN_TIME_NEGATIVE

/* Saves the current time in \buf. */
extern void sundry_time(sundry_time_t *buf);


SUNDRY_EXTERN_C_END
#endif /* SUNDRY_INCLUDED_sundry_time_h */

