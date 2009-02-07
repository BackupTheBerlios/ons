/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 7. February 2009
 */

/* ONS error handling.
 * Provides functions for reliable error handling.
 * Additionally, all error codes used in ONS are defined here.
 */

#include <ons/misc.h>

#ifndef ONS_INCLUDED_ons_main_h
#define ONS_INCLUDED_ons_main_h
ONS_EXTERN_C_BEGIN


#include <ons/sys.h>

/* Type used for ONS error codes. */
typedef uint64_t ons_err_t;

/* List of all error codes in ONS.
 * ONS_E_NONE is no error and signals that the function succeeded.
 * ONS_E_FAIL is returned if an unknown error occurred. If ONS was
 * compiled in debug mode, a message should also be printed to stderr.
 */
typedef enum ons_ecode_t {
    ONS_E_NONE = 0,
    ONS_E_SUCCESS = 0,
    ONS_E_DONE = 0,
    ONS_E_FAIL = 1,
    ONS_E_WSAFAIL                           /* WSA failed. */
} ons_ecode_t;

/* Windows error code definitions.
 * Due to the lack of POSIX error codes in Windows we wrap them
 * here. We wrap only the codes which are used in libraries
 * of ONS.
 * All error codes which are not available on Windows get an
 * randomly low (<0) number.
 * Defines the codes only if ONS_CONF_ECODES is defined (off by default).
 */
#if defined(ONS_CONF_WINDOWS) && defined(ONS_CONF_ECODES)
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


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_ons_main_h */

