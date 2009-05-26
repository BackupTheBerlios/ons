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

/* Thread wrapper.
 * Provides a generic interface for threading and locking
 * mechanisms.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "sundry/time.h"
#include "sundry/thread.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>


/* Starts a new thread. */
unsigned int sundry_thread_run(sundry_thread_t *thread, sundry_thread_handler_t function, void *arg) {
    SUNDRY_ASSERT(thread != NULL);
    SUNDRY_ASSERT(function != NULL);

#ifdef ONS_THREAD_PTHREAD
    if(pthread_create(thread, NULL, function, arg) != 0) return 0;
#elif defined(ONS_THREAD_WIN)
    *thread = CreateThread(NULL, 0, function, arg, 0, NULL);
    if(*thread == NULL) return 0;
#endif

    return 1;
}


/* Joins and then destructs a running or already stopped thread. */
void sundry_thread_join(sundry_thread_t *thread) {
    SUNDRY_ASSERT(thread != NULL);

#ifdef ONS_THREAD_PTHREAD
    pthread_join(*thread, NULL);
#elif defined(ONS_THREAD_WIN)
    WaitForSingleObject(*thread, INFINITE);
    CloseHandle(*thread);
#endif

    memset(thread, 0, sizeof(sundry_thread_t));
}


/* Initializes a new mutex. */
unsigned int sundy_mutex_init(sundry_mutex_t *mutex) {
    SUNDRY_ASSERT(mutex != NULL);

#ifdef ONS_THREAD_PTHREAD
    if(pthread_mutex_init(mutex, NULL) != 0) return 0;
#elif defined(ONS_THREAD_WIN)
    *mutex = CreateMutex(NULL, FALSE, NULL);
    if(*mutex == NULL) return 0;
#endif

    return 1;
}


/* Frees a mutex. */
void sundry_mutex_free(sundry_mutex_t *mutex) {
    SUNDRY_ASSERT(mutex != NULL);

#ifdef ONS_THREAD_PTHREAD
    pthread_mutex_destroy(mutex);
#elif defined(ONS_THREAD_WIN)
    CloseHandle(*mutex);
#endif
}


/* Unlocks a locked mutex. */
void sundry_mutex_unlock(sundry_mutex_t *mutex) {
    SUNDRY_ASSERT(mutex != NULL);

#ifdef ONS_THREAD_PTHREAD
    pthread_mutex_unlock(mutex);
#elif defined(ONS_THREAD_WIN)
    ReleaseMutex(*mutex);
#endif
}


void sundry_mutex_lock(sundry_mutex_t *mutex) {
    SUNDRY_ASSERT(mutex != NULL);

#ifdef ONS_THREAD_PTHREAD
    pthread_mutex_lock(mutex);
#elif defined(ONS_THREAD_WIN)
    WaitForSingleObject(*mutex, INFINITE);
#endif
}

unsigned int sundry_mutex_trylock(sundry_mutex_t *mutex) {
    SUNDRY_ASSERT(mutex != NULL);

#ifdef ONS_THREAD_PTHREAD
    return (pthread_mutex_trylock(mutex) == 0);
#elif defined(ONS_THREAD_WIN)
    return (WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0);
#endif
}

unsigned int sundry_mutex_timedlock(sundry_mutex_t *mutex, sundry_time_t *timeout) {
#ifdef ONS_THREAD_PTHREAD
    struct timespec absolute;
    sun_time_t current;
#endif

    SUNDRY_ASSERT(mutex != NULL);
    SUNDRY_ASSERT(timeout != NULL);

#ifdef ONS_THREAD_PTHREAD
    sundry_time(&current);
    absolute.tv_sec = current.secs + timeout->secs;
    /* \tv_nsec is 64bit wide, no overflow possible. */
    absolute.tv_nsec = (current.usecs + timeout->usecs) * 1000;

    /* Convert 10^9 nanoseconds into one second. */
    if(absolute.tv_nsec >= 1000000000) {
        absolute.tv_nsec -= 1000000000;
        ++absolute.tv_sec;
    }

    return (pthread_mutex_timedlock(mutex, &absolute) == 0);
#elif defined(ONS_THREAD_WIN)
    return (WaitForSingleObject(*mutex, (timeout->usec / 1000) + (timeout->secs * 1000)) == WAIT_OBJECT_0);
#endif
}

