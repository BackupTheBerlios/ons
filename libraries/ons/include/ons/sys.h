/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 7. February 2009
 */

/* ONS system wrapper header.
 * This header checks your system for specific functions and compatibility issues
 * and probably creates wrappers.
 */

#include <ons/misc.h>

#ifndef ONS_INCLUDED_ons_sys_h
#define ONS_INCLUDED_ons_sys_h
ONS_EXTERN_C_BEGIN


/* Check whether we have anonymus mmap and
 * Create MAP_ANONYMOUS if only MAP_ANON is declared.
 * If MAP_ANONYMOUS is not declared after this file is
 * included, your system does not support anonymous mmap().
 * You still need to include sys/mmap.h to use these constants.
 */
#ifdef ONS_CONF_HAVE_MMAP
    #ifndef ONS_CONF_HAVE_DECL_MAP_ANONYMOUS
        #ifdef ONS_CONF_HAVE_DECL_MAP_ANON
            #ifndef MAP_ANONYMOUS
                #define MAP_ANONYMOUS MAP_ANON
            #endif
        #endif
    #endif
#endif

/* Fixed size inetegers are heavily used in ONS.
 * We include this header here to allow the usage in
 * every other header without including it again.
 */
#include <stdint.h>

/* Declare boolean type.
 * _Bool is a builtin type in C99 and stdbool.h defines "bool"
 * as an alias.
 */
#include <stdbool.h>

/* Bitsets.
 * The variables represent bitsets with the given number of bits.
 */
typedef uint8_t ons_bitset8_t;
typedef uint16_t ons_bitset16_t;
typedef uint32_t ons_bitset32_t;
typedef uint64_t ons_bitset64_t;

/* Bit manipulation.
 * These macros manipulate the single bits of a value.
 * ONS_ROT(bit, val, k) rotates the \bit bit value \val for \k bytes to the left.
 * - ONS_ROT(32, 0x1, 4) would rotate the 32bit value 0x1 for 4 bits to the left and, hence, return 0x10.
 * ONS_BIT(x) returns a value with the \x'th bit set.
 */
#define ONS_ROT(bit, val, k) (( (val) << (k) ) | ( (val) >> ((bit) - (k)) ))
#define ONS_BIT(x) (0x1 << (x))

/* Comparison constants.
 * ONS_SMALLER if the compared element is smaller than the original.
 * ONS_EQUAL if the compared element is equal to the original.
 * ONS_GREATER if the compared element is greater than the original.
 */
typedef signed int ons_comp_t;
#define ONS_SMALLER -1
#define ONS_EQUAL 0
#define ONS_GREATER 1

/* Sleeps for \s seconds. */
extern void ons_sleep(unsigned int s);

/* Structure packing mechanism.
 * If __attribute__((__packed__)) is supported, we declare ONS_ATTR_PACK
 * to __attribute__((__packed__)).
 * Either this or #pragma pack is available.
 */
#ifdef ONS_CONF_HAVE_ATTR_PACKED
    #define ONS_ATTR_PACK __attribute__((__packed__))
#else
    #define ONS_ATTR_PACK
#endif

/* Network addressing has much to do with byte-ordering so we add here some global
 * macros helping you changing the byteorder of 2byte, 4byte and 8byte integers.
 */
#if defined(ONS_CONF_BIG_ENDIAN)
    #define ons_hton16(input) (((uint16_t)(input)))
    #define ons_hton32(input) (((uint32_t)(input)))
    #define ons_hton64(input) (((uint64_t)(input)))
    #define ons_ntoh16(input) (((uint16_t)(input)))
    #define ons_ntoh32(input) (((uint32_t)(input)))
    #define ons_ntoh64(input) (((uint64_t)(input)))
#elif defined(ONS_CONF_LITTLE_ENDIAN)
    #define ons_hton16(input) _ons_hton16(((uint16_t)(input)))
    #define _ons_hton16(input) ( \
                                    ((input & 0xff00) >> 8) | \
                                    ((input & 0x00ff) << 8) \
                               )
    #define ons_ntoh16(input) ons_hton16(input)

    #define ons_hton32(input) _ons_hton32(((uint32_t)(input)))
    #define _ons_hton32(input) ( \
                                    ((input & 0xff000000ULL) >> 24) | \
                                    ((input & 0x00ff0000ULL) >> 8) | \
                                    ((input & 0x0000ff00ULL) << 8) | \
                                    ((input & 0x000000ffULL) << 24) \
                               )
    #define ons_ntoh32(input) ons_hton32(input)

    #define ons_hton64(input) _ons_hton64(((uint64_t)(input)))
    #define _ons_hton64(input) ( \
                                    ((input & 0xff00000000000000ULL) >> 56) | \
                                    ((input & 0x00ff000000000000ULL) >> 40) | \
                                    ((input & 0x0000ff0000000000ULL) >> 24) | \
                                    ((input & 0x000000ff00000000ULL) >> 8) | \
                                    ((input & 0x00000000ff000000ULL) << 8) | \
                                    ((input & 0x0000000000ff0000ULL) << 24) | \
                                    ((input & 0x000000000000ff00ULL) << 40) | \
                                    ((input & 0x00000000000000ffULL) << 56) \
                               )
    #define ons_ntoh64(input) ons_hton64(input)
#endif

/* Contains a timestamp in seconds and microseconds.
 * \t_sec contains the seconds since the EPOCH and
 * \t_usec contains the microseconds since the last second.
 * If the system does not support microseconds but milliseconds,
 * the \t_usec is appended with "000".
 * If the system does not supported any mechanism to get milliseconds
 * the \t_usec contains always 0.
 *
 * This structure does not support negative values. Though, some systems
 * support negative values in time_t, we do not. This disallows weird
 * behaviour if the function does not check this. If you need negative
 * values AND positive values, you should pass an additional parameter.
 */
typedef struct ons_time_t {
    uint64_t t_sec;
    uint64_t t_usec;
} ons_time_t;

/* Saves the current time in \buf. */
extern void ons_time(ons_time_t *buf);

/* Thread wrapper.
 * Provides a system independent API to thread functionality. Though, the datatypes
 * behind the typedefs are not equal on all machines which does not conform to ONS
 * guidelines, this makes the wrapper much faster and we do not need to create
 * extra functions.
 */
#ifdef ONS_CONF_THREADED
    #ifdef ONS_CONF_WINDOWS
        typedef HANDLE ons_thread_t;
        typedef HANDLE ons_mutex_t;
    #elif defined(ONS_CONF_HAVE_PTHREAD_H)
        typedef pthread_t ons_thread_t;
        typedef pthread_mutex_t ons_mutex_t;
    #endif
    typedef void *(*ons_thread_handler_t)(void *arg);
    extern bool ons_thread_run(ons_thread_t *thread, ons_thread_handler_t function, void *arg);
    extern void ons_thread_join(ons_thread_t *thread);
    extern bool ons_mutex_init(ons_mutex_t *mutex);
    extern void ons_mutex_free(ons_mutex_t *mutex);
    extern void ons_mutex_lock(ons_mutex_t *mutex);
    extern void ons_mutex_unlock(ons_mutex_t *mutex);
    extern bool ons_mutex_trylock(ons_mutex_t *mutex);
    extern bool ons_mutex_timedlock(ons_mutex_t *mutex, ons_time_t *timeout);
#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_ons_sys_h */

