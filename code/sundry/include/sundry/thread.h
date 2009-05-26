/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 13. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 25. May 2009
 */

/* Thread wrapper.
 * Provides a generic interface for threading and locking
 * mechanisms.
 */


#include <sundry/sundry.h>

#ifndef SUNDRY_INCLUDED_sundry_thread_h
#define SUNDRY_INCLUDED_sundry_thread_h
SUNDRY_EXTERN_C_BEGIN


#include <sundry/time.h>


/* Define the datatype for a thread object and mutexes. */
#if defined(ONS_THREAD_PTHREAD)
    typedef pthread_t sundry_thread_t;
    typedef pthread_mutex_t sundry_mutex_t;
#elif defined(ONS_THREAD_WIN)
    typedef HANDLE sundry_thread_t;
    typedef HANDLE sundry_mutex_t;
#else
    #error "No thread backend specified. Please check your ONS configuration."
#endif

/* The type of the function which is started as a new thread. */
typedef void (*sundry_thread_handler_t)(void *arg);

/* This starts a new thread.
 * \thread must point to an uninitialized user-allocated \sundry_thread_t object.
 * \function must be a valid function which is started in the new thread.
 * \arg is directly passed to \function when it is started.
 * Returns 1 when the thread was successfully started and 0 when the process is not allowed
 * to create more threads due to a lack of needed resources.
 */
extern unsigned int sundry_thread_run(sundry_thread_t *thread, sundry_thread_handler_t function, void *arg);

/* This waits for a running thead.
 * \thread must point to an object which was previously initialized with \sundry_thread_run.
 */
extern void sundry_thread_join(sundry_thread_t *thread);

/* Initializes a new mutes. Returns 0 when the process is not allowed to create further mutexes,
 * otherwise it returns 1.
 */
extern unsigned int sundry_mutex_init(sundry_mutex_t *mutex);
/* Frees all space allocated by a mutex. */
extern void sundry_mutex_free(sundry_mutex_t *mutex);
/* This unlocks a previously locked mutex. */
extern void sundry_mutex_unlock(sundry_mutex_t *mutex);
/* Locks a mutex. This call blocks until the mutex could be locked by the calling thread. */
extern void sundry_mutex_lock(sundry_mutex_t *mutex);
/* Locks a mutex. This immediately returns when another thread already holds the lock.
 * Returns 1 when the mutex could be locked, otherwise 0.
 */
extern unsigned int sundry_mutex_trylock(sundry_mutex_t *mutex);
/* Locks a mutex. This function returns when the timeout \timeout has exceeded and the mutex
 * could not be locked.
 * Returns 1 when the mutex has been successfully locked, otherwise it returns 0 after the timeout exceeded.
 */
extern unsigned int sundry_mutex_timedlock(sundry_mutex_t *mutex, sundry_time_t *timeout);


SUNDRY_EXTERN_C_END
#endif /* SUNDRY_INCLUDED_sundry_thread_h */

