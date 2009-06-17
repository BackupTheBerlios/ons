/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 13. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 17. June 2009
 */

/* Time controlling API.
 * This API is similar to <time.h> but is much more precise.
 * It stores seconds and microseconds in two separate integers,
 * however, this way it is possible that the microseconds int
 * is bigger or equal than 1,000,000, that is, it should be
 * converted into seconds.
 * All functions here automatically return "valid" times where
 * this does not occur, however, if you manually modify them
 * please go sure to "normalize" them.
 * For common math operations this header provides functions
 * that take this in mind.
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
    uint32_t usecs;
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

/* Computes the time difference between \ctime and \ptime and saves
 * it into \buf.
 * \ctime and \ptime have to be valid timestamps.
 */
extern void sundry_timediff(sundry_time_t *buf, sundry_time_t *ctime, sundry_time_t *ptime);

/* Sums up two times and stores them into \buf.
 * Both times have to be valid timestamps.
 */
extern void sundry_timesum(sundry_time_t *buf, sundry_time_t *ctime, sundry_time_t *ptime);

/* Sleeps the process for the specified amount of time. */
extern void sundry_sleep(sundry_time_t *ptime);


SUNDRY_EXTERN_C_END
#endif /* SUNDRY_INCLUDED_sundry_time_h */

