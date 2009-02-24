/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
 */

/* ONS error handling.
 * Provides functions for reliable error handling.
 * Additionally, all error codes used in ONS are defined here.
 */

#include <ons/misc.h>

#ifndef ONS_INCLUDED_ons_err_h
#define ONS_INCLUDED_ons_err_h
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

    /* General error codes.
     * Some of these errors have no single meaning. Please see each function for the meaning of the error code.
     */
    ONS_E_WSAFAIL,              /* WSA failed. */
    ONS_E_MEMFAIL,              /* Kernel denied further object related memory. */
    ONS_E_UNKNOWN,              /* Unknown parameter. */
    ONS_E_BADARG,               /* In contrast to ONS_E_UNKNOWN this is returned if the parameters are invalid (but known). */
    ONS_E_DENIED,               /* Access denied. */
    ONS_E_SIGFLOOD,             /* A signal interrupted the syscall. */
    ONS_E_OPNOTSUPP,            /* Operation not supported. */
    ONS_E_NONBLOCK,             /* Either operation is continued in background or the operation is aborted 'cause it would block. */
    ONS_E_BADFD,                /* The file descriptor is not valid (already closed) or not a socket. */
    ONS_E_ALREADY,              /* Operation already in progress. */
    ONS_E_AGAIN,                /* Try again. */

    /* Address family or protocol [family] error.
     * These errors describe some failures when an address family, a protocol family or a transmission method is used
     * in a wrong manner or is not supported on your platform in this constelation.
     * Remember, "protocol" and "protocol family" are two totally different things.
     */
    ONS_E_AFNOTSUPP,            /* Address family not supported on this machine. */
    ONS_E_PFNOTSUPP,            /* Protocol family not supported on this machine. */
    ONS_E_PROTONOTSUPP,         /* Protocol or transmission method not supported on this machine. */
    ONS_E_PROTODOM,             /* This protocol is not supported within this transmission method, or this transmission method is not supported within this domain. */
    ONS_E_MISSING,              /* Missing kernel package. */

    /* Address errors. */
    ONS_E_ADDRINUSE,            /* Address already in use. */
    ONS_E_ADDRNOTAVAIL,         /* The address is not available on this machine. */
    ONS_E_INVALADDR,            /* Invalid/Unknown address. */
    ONS_E_DESTREQ,              /* Destination address required. */

    /* Errors occurring during a connection attempt. */
    ONS_E_NETDOWN,              /* Network is down. */
    ONS_E_NETUNREACH,           /* Network unreachable. */
    ONS_E_HOSTDOWN,             /* Host is down. */
    ONS_E_HOSTUNREACH,          /* Host is unreachable. */
    ONS_E_ISCONN,               /* Already connected. */
    ONS_E_REFUSED,              /* Connection refused by other side. */
    ONS_E_TIMEDOUT,             /* Operation timed out. */

    /* Connection reset. */
    ONS_E_NETRESET,             /* Connection closed due to network reset. */
    ONS_E_RESET,                /* Connection reset by peer. */
    ONS_E_CONNABORT,            /* The connection has been aborted by our side. */

    /* IO errors. */
    ONS_E_SHUTDOWN,             /* Connection already shutdown. */
    ONS_E_PIPE,                 /* Local end has already been closed. */
    ONS_E_NOTCONN,              /* Socket is not connected, yet. */
    ONS_E_MSGSIZE,              /* Message size too big. */
    ONS_E_DROP,                 /* Packet dropped due to queue overflow (most time packages are dropped silently). */

    /* Socket creation/binding errors. */
    ONS_E_MFILE,                /* Process file descriptor table overflow. */
    ONS_E_NFILE,                /* Global file descriptor table overflow. */
    ONS_E_PROVERFLOW,           /* Port number or routing cache overflow. */
    ONS_E_PROTOFAIL,            /* Protocol error. */
    ONS_E_INVALIDDEV,           /* Invalid device. */

    /* Option setting errors. */
    ONS_E_NOPROTOOPT,           /* The option is unknown at the level indicated. */

    /* Errors which occur only with Unix Sockets. */
    ONS_E_ISDIR,                /* Is a directory. */
    ONS_E_TOOLARGE,             /* File is too large. */
    ONS_E_NOSPACELEFT,          /* No space left on device. */
    ONS_E_QUOTA,                /* Quota exceeded. */
    ONS_E_LOOP,                 /* Too many symlinks encountered in local address. */
    ONS_E_NAMETOOLONG,          /* The local address has an overlength. */
    ONS_E_NOFILE,               /* File does not exist. */
    ONS_E_NOTDIR,               /* Invalid directory path. */
    ONS_E_ROFS,                 /* Read only filesystem. */
    ONS_E_IO,                   /* IO failure. */

    /* End of list indicator. Always the highest error number. */
    ONS_E_LAST
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
#endif /* ONS_INCLUDED_ons_err_h */

