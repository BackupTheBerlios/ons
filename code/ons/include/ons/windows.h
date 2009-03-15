/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 15. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* WINDOWS specific API description
 * We describe the WIN32 and WIN64 API here. We try to support
 * all aspects of windows.
 * However, WIN16 support is not included.
 *
 * David Herrmann -- 2009/03/15
 */


#ifndef ONS_INCLUDED_ons_linux_h
#define ONS_INCLUDED_ons_linux_h
#ifdef __cplusplus
extern 'C' {
#endif


/* API: Machine
 * We compile the constants out of machine.h defined ONS_CONF_* constants. The
 * machine.h header is already included in ons.h.
 */
#define ONS_MACHINE_LITTLEENDIAN
#define ONS_MACHINE_PACK

#ifndef ONS_CONF_NO_IPV6
    #define ONS_MACHINE_IPV6
#endif

#ifdef ONS_CONF_DEBUG
    #define ONS_MACHINE_DEBUG 5
#endif


/* API: System
 * This API is required and must be supported on all systems.
 */
#define ONS_SYSTEM
#include <stdint.h>
#include <windows.h>
#include <winsock2.h>


/* API: String
 * This API is available on windows machines, however, strnlen is not
 * available.
 */
#define ONS_STRING
/* #define ONS_STRING_STRNLEN */


/* Windows error code definitions.
 * Due to the lack of POSIX error codes in Windows we wrap them
 * here. We wrap only the codes which are used in libraries
 * of ONS.
 * All error codes which are not available on Windows get an
 * randomly low (<0) number.
 * Defines the codes only if ONS_DEF_ECODES is defined (off by default).
 */
#if defined(ONS_DEF_ECODES)
    #define EPERM WSAEACCESS
    #define EACCESS WSAEACCESS
    #define EINTR WSAEINTR
    #define EBADF WSAEBADF
    #define ENOTSOCK WSAENOTSOCK
    #define ENOMEM WSA_NOT_ENOUGH_MEMORY
    #define ENOBUFS WSAENOBUFS
    #define EMFILE WSAEMFILE
    #define EFAULT WSAEFAULT
    #define EINVAL WSAEINVAL
    #define EINPROGRESS WSAEINPROGRESS
    #define EWOULDBLOCK WSAEWOULDBLOCK
    #define EAGAIN EWOULDBLOCK
    #define EALREADY WSAEALREADY
    #define EDESTADDRREQ WSAEDESTADDRREQ
    #define ENOPROTOOPT WSAENOPROTOOPT
    #define EPROTONOSUPPORT WSAEPROTONOSUPPORT
    #define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
    #define EPROTOTYPE WASEPROTOTYPE
    #define EOPNOTSUPP WSAEOPNOTSUPP
    #define EPFNOSUPPORT WSAEPFNOSUPPORT
    #define EAFNOSUPPORT WSAEAFNOSUPPORT
    #define EADDRINUSE WSAEADDRINUSE
    #define EADDRNOTAVAIL WSAEADDRNOTAVAIL
    #define ENETDOWN WSAENETDOWN
    #define ENETUNREACH WSAENETUNREACH
    #define ENETRESET WSAENETRESET
    #define ECONNABORTED WSAECONNABORT
    #define ECONNRESET WSAECONNRESET
    #define ESHUTDOWN WSAESHUTDOWN
    #define EISCONN WSAEISCONN
    #define ENOTCONN WSAENOTCONN
    #define ETIMEDOUT WSAETIMEDOUT
    #define ECONNREFUSED WSAECONNREFUSED
    #define EHOSTDOWN WSAEHOSTDOWN
    #define EHOSTUNREACH WSAEHOSTUNREACH
    #define EMSGSIZE WSAEMSGSIZE
    #define EPIPE WSAEDISCON
    #define EDQUOT WSAEUSERS
    #define ENOSPC WSAEDQUOT
    #define ELOOP WSAELOOP
    #define ENAMETOOLONG WSAENAMETOOLONG
    #define ENFILE -1
    #define ENOPKG -2
    #define ENXIO -3
    #define ENODEV -4
    #define EPROTO -5
    #define EIO -6
    #define ENOENT -7
    #define ENOTDIR -8
    #define EISDIR -9
    #define EMLINK -10
    #define EFBIG -11
    #define EROFS -12
#endif


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_linux_h */

