/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
 */

/* This file contains global declarations for all libraries
 * which are part of ONS.
 *
 * If you want to configure ONS for your system which does not use autoconf,
 * please read the comments in this file carefully and edit the file at will.
 */

#ifndef ONS_INCLUDED_ons_misc_h
#define ONS_INCLUDED_ons_misc_h
#ifdef __cplusplus
extern 'C' {
#endif


/** Due to the lack of autoconf on some systems like Windows, we need to define
 * the constants manually. Please modify the values to fit to your system.
 * The code below this block is ONS code. Do not modify it.
 *
 * You only have to modify the section for your system. The other sections
 * are ignored at compile time. If no system dependent configuration is
 * available, the autoconf header is taken.
 * Please use the autotools if they are available on your system. They check
 * your system for additional preconditions which cannot be done here.
 *
 * Following a list of all available constants:
 * - Either ONS_CONF_BIG_ENDIAN or ONS_CONF_LITTLE_ENDIAN must be defined.
 * - ONS_CONF_HAVE_MMAP is defined if mmap() and sys/mmap.h are available.
 * - ONS_CONF_HAVE_DECL_MAP_ANON is defined if MAP_ANON is defined in sys/mmap.h.
 * - ONS_CONF_HAVE_DECL_MAP_ANONYMOUS is defined if MAP_ANONYMOUS is defined in sys/mmap.h.
 * - ONS_CONF_HAVE_STRNLEN is defined if strnlen() is available in your Standard-C-Library.
 * - ONS_CONF_HAVE_SYS_UN_H is defined if sys/un.h is available.
 * - ONS_CONF_HAVE_SYS_SOCKET_H is defined if sys/socket.h is available.
 * - ONS_CONF_HAVE_NETINET_IN_H is defined if netinet/in.h is available.
 * - ONS_CONF_HAVE_UNISTD_H is defined if unistd.h is available.
 * - ONS_CONF_HAVE_UNIX_SOCKETS is defined if file/unix sockets aka "struct sockaddr_un" are available.
 * - ONS_CONF_PATH_SIZE is defined to the size of the "sun_path" member of "struct sockaddr_un" if available.
 * - ONS_CONF_HAVE_SA_LEN is defined if "struct sockaddr" has a "sa_len" member.
 * - ONS_CONF_HAVE_SIN_LEN is defined if "struct sockaddr_in" has a "sin_len" member.
 * - ONS_CONF_HAVE_SIN6_LEN is defined if "struct sockaddr_in6" has a "sin6_len" member.
 * - ONS_CONF_HAVE_SUN_LEN is defined if "struct sockaddr_un" has a "sun_len" member if available.
 * - ONS_CONF_HAVE_ATTR_PACKED is defined if __attribute__((__packed__)) is supported.
 * - ONS_CONF_HAVE_PRAGMA_PACK is defined if "#pragma pack" is supported.
 * - ONS_CONF_NO_IPV6 is defined if all IPv6 support should be disabled.
 * - ONS_CONF_HAVE_SYS_IOCTL_H is defined if sys/ioctl.h is available.
 * - ONS_CONF_HAVE_FCNTL_H is defined if fcntl.h is available.
 * - ONS_CONF_HAVE_SYS_TYPES_H is defined if sys/types.h is available.
 * - ONS_CONF_HAVE_EXT_SOCKET is defined if socket() supports bitflags as second parameter.
 * - ONS_CONF_HAVE_ACCEPT4 is defined if accept4() is available.
 * - ONS_CONF_NONBLOCK_POSIX is defined if POSIX socket nonblocking mode is available.
 * - ONS_CONF_NONBLOCK_BSD is defined if BSD socket nonblocking mode is available.
 * - ONS_CONF_NONBLOCK_SYSV is defined if SYSV socket nonblocking mode is available.
 * - ONS_CONF_HAVE_GETTIMEOFDAY is defined if gettimeofday() and sys/time.h are available.
 * - ONS_CONF_HAVE_PTHREAD_H is defined if pthread.h is available and the library is linked to -lpthread.
 * - ONS_CONF_WINDOWS is defined on windows machines.
 * - ONS_CONF_DEBUG is defined if ONS is compiled in debug mode.
 */
/* Modify the appropriate block here. */
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS_)
    /* WINDOWS 32Bit and 64Bit machines. */
    #define ONS_CONF_LITTLE_ENDIAN
    #define ONS_CONF_HAVE_PRAGMA_PACK
    #define ONS_CONF_WINDOWS
#else
    /* Use autoconf if nothing is predefined. */
    #include <ons/setup.h>
#endif

/* NULL must be defined in every header we use, thus, include stddef.h. */
#include <stddef.h>

/* To make all headers available in C++ they need to define "extern 'C'".
 * This macros defines "extern 'C'" if the compiler is a C++ compiler.
 * At the end of the header you must close "extern 'C'".
 *
 * ONS_EXTERN_C: Defined as "extern 'C'" if _cplusplus is defined.
 * ONS_EXTERN_C_BEGIN: Defined as "extern 'C' {"if _cplusplus is defined.
 * ONS_EXTERN_C_END: Defined as "}" if _cplusplus is defined.
 */
#ifdef __cplusplus
    #define ONS_EXTERN_C extern 'C'
    #define ONS_EXTERN_C_BEGIN ONS_EXTERN_C {
    #define ONS_EXTERN_C_END }
#else
    #define ONS_EXTERN_C
    #define ONS_EXTERN_C_BEGIN
    #define ONS_EXTERN_C_END
#endif

#ifdef ONS_CONF_DEBUG
    #define ONS_DEBUG ONS_CONF_DEBUG
#endif

/* Include windows.h and winsock2.h headers on Windows. */
#ifdef ONS_CONF_WINDOWS
    #include <windows.h>
    #include <winsock2.h>
#endif

/* Include pthread if desired. */
#if defined(ONS_CONF_HAVE_PTHREAD_H)
    #include <pthread.h>
#endif

#if defined(ONS_CONF_HAVE_PTHREAD_H) || defined(ONS_CONF_WINDOWS)
    #define ONS_CONF_THREADED
#endif

/* We use assert() everywhere. Include it here! */
#include <assert.h>

/* Function which is called on fatal error.
 * Calls abort().
 * If \msg is NULL, a default msg is printed.
 */
extern void ons_fatal_error(const char *format, ...);
#define ONS_ABORT(msg) ons_fatal_error("ONS failed in %s at %u: %s\n", __FILE__, __LINE__, msg)
#ifdef ONS_DEBUG
    #define ONS_ASSERT(x) ((x)?ONS_ABORT("Assertation failed."):0)
#else
    #define ONS_ASSERT(x)
#endif


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_misc_h */

