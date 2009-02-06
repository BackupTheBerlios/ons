/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. July 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Error handling.
 * This file provides an error handling interface for the FIP library.
 */

#include <fip/misc.h>

#ifndef ONS_INCLUDED_fip_error_h
#define ONS_INCLUDED_fip_error_h
ONS_EXTERN_C_BEGIN


/* Error codes (unsigned int) */
typedef enum fip_error_code_t {
    /* Start of error list. The lowest number of all.
     * Indicates success.
     */
    FIP_E_NONE = 0,
    #define FIP_E_FIRST FIP_E_NONE

    /* General error codes.
     * Some of these errors have no single meaning. Please see each function for the meaning of the error code.
     */
    FIP_E_FAIL,                 /* Unknown failure or unexpected error. If debug is enabled, a stderr-message should be printed. */
    FIP_E_MEMFAIL,              /* Memory allocation failed (mostly kernel internal buffer). */
    FIP_E_UNKNOWN,              /* Unknown parameter. */
    FIP_E_BADARG,               /* In contrast to FIP_E_UNKNOWN this is returned if the parameters are invalid (but known). */
    FIP_E_DENIED,               /* Access denied. */
    FIP_E_SIGFLOOD,             /* A signal interrupted the syscall. */
    FIP_E_OPNOTSUPP,            /* Operation not supported. */
    FIP_E_NONBLOCK,             /* Either operation is continued in background or the operation is aborted 'cause it would block. */
    FIP_E_BADFD,                /* The file descriptor is not valid (already closed) or not a socket. */
    FIP_E_ALREADY,              /* Operation already in progress. */
    FIP_E_AGAIN,                /* Try again. */

    /* Address family or protocol [family] error.
     * These errors describe some failures when an address family, a protocol family or a transmission method is used
     * in a wrong manner or is not supported on your platform in this constelation.
     * Remember, "protocol" and "protocol family" are two totally different things.
     */
    FIP_E_AFNOTSUPP,            /* Address family not supported on this machine. */
    FIP_E_PFNOTSUPP,            /* Protocol family not supported on this machine. */
    FIP_E_PROTONOTSUPP,         /* Protocol or transmission method not supported on this machine. */
    FIP_E_PROTODOM,             /* This protocol is not supported within this transmission method, or this transmission method is not supported within this domain. */
    FIP_E_MISSING,              /* Missing kernel package. */

    /* Address errors. */
    FIP_E_ADDRINUSE,            /* Address already in use. */
    FIP_E_ADDRNOTAVAIL,         /* The address is not available on this machine. */
    FIP_E_INVALADDR,            /* Invalid/Unknown address. */
    FIP_E_DESTREQ,              /* Destination address required. */

    /* Errors occurring during a connection attempt. */
    FIP_E_NETDOWN,              /* Network is down. */
    FIP_E_NETUNREACH,           /* Network unreachable. */
    FIP_E_HOSTDOWN,             /* Host is down. */
    FIP_E_HOSTUNREACH,          /* Host is unreachable. */
    FIP_E_ISCONN,               /* Already connected. */
    FIP_E_REFUSED,              /* Connection refused by other side. */
    FIP_E_TIMEDOUT,             /* Operation timed out. */

    /* Connection reset. */
    FIP_E_NETRESET,             /* Connection closed due to network reset. */
    FIP_E_RESET,                /* Connection reset by peer. */
    FIP_E_CONNABORT,            /* The connection has been aborted by our side. */

    /* IO errors. */
    FIP_E_SHUTDOWN,             /* Connection already shutdown. */
    FIP_E_PIPE,                 /* Local end has already been closed. */
    FIP_E_NOTCONN,              /* Socket is not connected, yet. */
    FIP_E_MSGSIZE,              /* Message size too big. */
    FIP_E_DROP,                 /* Packet dropped due to queue overflow (most time packages are dropped silently). */

    /* Socket creation/binding errors. */
    FIP_E_MFILE,                /* Process file descriptor table overflow. */
    FIP_E_NFILE,                /* Global file descriptor table overflow. */
    FIP_E_PROVERFLOW,           /* Port number or routing cache overflow. */
    FIP_E_PROTOFAIL,            /* Protocol error. */
    FIP_E_INVALIDDEV,           /* Invalid device. */

    /* Option setting errors. */
    FIP_E_NOPROTOOPT,           /* The option is unknown at the level indicated. */

    /* Errors which occur only with Unix Sockets. */
    FIP_E_ISDIR,                /* Is a directory. */
    FIP_E_TOOLARGE,             /* File is too large. */
    FIP_E_NOSPACELEFT,          /* No space left on device. */
    FIP_E_QUOTA,                /* Quota exceeded. */
    FIP_E_LOOP,                 /* Too many symlinks encountered in local address. */
    FIP_E_NAMETOOLONG,          /* The local address has an overlength. */
    FIP_E_NOFILE,               /* File does not exist. */
    FIP_E_NOTDIR,               /* Invalid directory path. */
    FIP_E_ROFS,                 /* Read only filesystem. */
    FIP_E_IO,                   /* IO failure. */

    /* End of list indicator. Always the highest error number. */
    FIP_E_LAST
} fip_error_code_t;

/* Error classes */
#define FIP_C_NONE              0x0000 /* No class. */
/* base classes */
#define FIP_C_GENERAL           0x0001
#define FIP_C_MISSING           0x0002
#define FIP_C_ADDRESS           0x0004
#define FIP_C_CONNECTING        0x0008
#define FIP_C_SHUTDOWN          0x0010
#define FIP_C_IO                0x0020
#define FIP_C_BINDING           0x0040
#define FIP_C_OPTION            0x0080
#define FIP_C_FILE              0x0100
/* additional classes */
#define FIP_C_CRITICAL          0x0200 /* There is no general rule which errors are "critical"
                                        * except that you should treat them separately/specially.
                                        */
#define FIP_C_CLOSED            0x0400 /* Socket is closed now. */
#define FIP_C_TEMPORARY         0x0800 /* Temporary failures (same constellation could succeed). */
#define FIP_C_FATAL             0x1000 /* Constellation which will never succeed in this process. */

typedef struct fip_einfo_t {
    unsigned int eclass;
    const char *emsg;
} fip_einfo_t;

/* Error code information */
extern const fip_einfo_t const fip_error_info[FIP_E_LAST];

/* Returns the classes an error code is member of. */
#define FIP_ECLASS(ecode) (fip_error_info[(ecode)].eclass)

/* Returns 1 if an ERROR CODE is part of a class, otherwise 0. */
#define FIP_EISOF(ecode, eclass) (FIP_ECLASS(ecode) & eclass)

/* Takes an internal system dependent error code and returns the
 * FIP equivalent. Not every FIP error has one single equivalent.
 * It is kinda more complicated.
 *
 * _file: recognizes only filesystem related error codes.
 */
extern fip_err_t fip_mkerrno(signed int err);
extern fip_err_t fip_mkerrno_file(signed int err);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_error_h */

