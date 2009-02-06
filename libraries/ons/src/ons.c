/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 2. January 2009
 */

/* Main source file of ONS.
 * This file implements main functionality used in every library which is
 * part of ONS.
 * Only really crucial functions/variables are implemented here.
 */

#include "ons/ons.h"
#include <errno.h>
#include <time.h>

#ifdef ONS_CONF_HAVE_GETTIMEOFDAY
    #include <sys/time.h>
#endif

/* Initializes the core library. Does not have to be threadsafe.
 * Returns 0 on success, otherwise an error code.
 */
signed int ons_init(ons_bitset8_t opts) {
    static unsigned int called = 0;

    if(called) return 0;

#ifdef ONS_CONF_WINDOWS
    if(opts & ONS_INIT_WSA) {
        signed long ret;
        WSADATA lwsa_data;

        ret = WSAStartup(MAKEWORD(2, 0), &lwsa_data);
        if(ret) return ONS_E_WSAFAIL;
    }
#endif

    called = 1;
    return ONS_E_NONE;
}

/* Deinitializes the core library. Does not have to be threadsafe. */
void ons_deinit(ons_bitset8_t opts) {
#ifdef ONS_CONF_WINDOWS
    if(opts & ONS_INIT_WSA) WSACleanup();
#endif
}

void ons_time(ons_time_t *buf) {
#ifdef GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    buf->t_sec = tv.tv_sec;
    buf->t_usec = tv.tv_usec;
#elif defined(ONS_CONF_WINDOWS)
    /* Did I already mention that this really sucks? */
    FILETIME o_filetime;
    uint64_t o_utime = 0;

    /* Get the systems time. */
    GetSystemTimeAsFileTime(&o_filetime);

    /* Assemble the real time with the data we got. */
    o_utime |= o_filetime.dwHighDateTime;
    o_utime <<= 32;
    o_utime |= o_filetime.dwLowDateTime;
 
    /* Converting file time to unix epoch.
     * Wooouuh, this is so slow that it almost makes the whole
     * microseconds issue totally useless.
     * Perhaps, due to this Windows does only return milliseconds
     * and not microseconds, haha.
     */
    o_utime /= 10;
    o_utime -= 11644473600000000ULL;
    buf->t_sec = (o_utime / 1000000ULL);
    buf->t_usec = (o_utime % 1000000ULL);
#else
    buf->t_usec = 0;
    buf->t_sec = time(NULL);
#endif
}

#ifdef ONS_CONF_WINDOWS
    bool ons_thread_run(ons_thread_t *thread, ons_thread_handler_t function, void *arg) {
        *thread = CreateThread(NULL, 0, function, arg, 0, NULL);
        return (*thread != NULL);
    }

    void ons_thread_join(ons_thread_t *thread) {
        WaitForSingleObject(*thread, INFINITE);
        CloseHandle(*thread);
    }

    bool ons_mutex_init(ons_mutex_t *mutex) {
        *mutex = CreateMutex(NULL, FALSE, NULL);
        return (*mutex != NULL);
    }

    void ons_mutex_free(ons_mutex_t *mutex) {
        CloseHandle(*mutex);
    }

    void ons_mutex_lock(ons_mutex_t *mutex) {
        WaitForSingleObject(*mutex, INFINITE);
    }

    void ons_mutex_unlock(ons_mutex_t *mutex) {
        ReleaseMutex(*mutex);
    }

    bool ons_mutex_trylock(ons_mutex_t *mutex) {
        return (WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0);
    }

    bool ons_mutex_timedlock(ons_mutex_t *mutex, ons_time_t *timeout) {
        return (WaitForSingleObject(*mutex, (timeout.tv_usec / 1000) + (timeout.tv_sec * 1000)) == WAIT_OBJECT_0);
    }
#elif defined(ONS_CONF_HAVE_PTHREAD_H)
    bool ons_thread_run(ons_thread_t *thread, ons_thread_handler_t function, void *arg) {
        pthread_attr_t attribute;
        signed int ret;

        /* Create attributes and set thread joinable. */
        if(pthread_attr_init(&attribute) != 0) return false;
        pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);

        /* Launch the threaded function. */
        ret = pthread_create(thread, &attribute, function, arg);

        /* Cleanup. */
        pthread_attr_destroy(&attribute);
        return (ret == 0);
    }

    void ons_thread_join(ons_thread_t *thread) {
        pthread_join(*thread, NULL);
    }

    bool ons_mutex_init(ons_mutex_t *mutex) {
        return (pthread_mutex_init(mutex, NULL) == 0);
    }

    void ons_mutex_free(ons_mutex_t *mutex) {
        pthread_mutex_destroy(mutex);
    }

    void ons_mutex_lock(ons_mutex_t *mutex) {
        pthread_mutex_lock(mutex);
    }

    void ons_mutex_unlock(ons_mutex_t *mutex) {
        pthread_mutex_unlock(mutex);
    }

    bool ons_mutex_trylock(ons_mutex_t *mutex) {
        return (pthread_mutex_trylock(mutex) == 0);
    }

    bool ons_mutex_timedlock(ons_mutex_t *mutex, ons_time_t *timeout) {
        struct timespec absolute;
        ons_time_t current;

        ons_time(&current);
        absolute.tv_sec = current.t_sec + timeout->t_sec;
        /* \tv_nsec is 64bit wide, there is not overflow possible. */
        absolute.tv_nsec = (current.t_usec + timeout->t_usec) * 1000;

        /* Convert 10^9 nanoseconds into one second. */
        if(absolute.tv_nsec >= 1000000000) {
            absolute.tv_nsec -= 1000000000;
            ++absolute.tv_sec;
        }

        return (pthread_mutex_timedlock(mutex, &absolute) == 0);
    }
#endif

