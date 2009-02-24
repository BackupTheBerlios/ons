/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. June 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
 */

/* Provides reliable socket error handling.
 * Most Unixes have different socket errors. This file catches them all and returns a unique FIP error code.
 * These functions are used inside the FIP socket functions and should not be used outside of them.
 *
 * This file uses sometimes the system dependent errors (saved in a SIGNED INT), thus please use
 * them only if you know what you do.
 */

#define ONS_CONF_ECODES
#include "fip/fip.h"
#include <errno.h>

/* Error code information. Although this system provides an error message, you should not use them, because
 * they are neither the same on all functions nor are they localized.
 */
const fip_einfo_t const fip_error_info[ONS_E_LAST] = {
    /* ONS_E_NONE */            { FIP_C_NONE, "Operation succeeded." },
    /* ONS_E_FAIL */            { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Unknown failure." },
    /* ONS_E_MEMFAIL */         { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Memory allocation failed." },
    /* ONS_E_UNKNOWN */         { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Unknown parameter." },
    /* ONS_E_BADARG */          { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Invalid parameter." },
    /* ONS_E_DENIED */          { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Access denied." },
    /* ONS_E_SIGFLOOD */        { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_TEMPORARY, "A signal interrupted the syscall." },
    /* ONS_E_OPNOTSUPP */       { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_FATAL, "Operation not supported." },
    /* ONS_E_NONBLOCK */        { FIP_C_GENERAL, "Either operation is continued in background or the operation is aborted because it would block." },
    /* ONS_E_BADFD */           { FIP_C_GENERAL | FIP_C_CLOSED, "The file descriptor is not valid or not a socket." },
    /* ONS_E_ALREADY */         { FIP_C_GENERAL, "Operation already in progress." },
    /* ONS_E_AGAIN */           { FIP_C_GENERAL | FIP_C_CRITICAL | FIP_C_TEMPORARY, "Try again." },
    /* ONS_E_AFNOTSUPP */       { FIP_C_MISSING | FIP_C_CRITICAL | FIP_C_FATAL, "Address family not supported on this machine." },
    /* ONS_E_PFNOTSUPP */       { FIP_C_MISSING | FIP_C_CRITICAL | FIP_C_FATAL, "Protocol family not supported on this machine." },
    /* ONS_E_PROTONOTSUPP */    { FIP_C_MISSING | FIP_C_CRITICAL | FIP_C_FATAL, "Protocol or transmission method not supported on this machine." },
    /* ONS_E_PROTODOM */        { FIP_C_MISSING | FIP_C_CRITICAL | FIP_C_FATAL, "This protocol is not supported within this transmission method, or this transmission method is not supported within this domain." },
    /* ONS_E_MISSING */         { FIP_C_MISSING | FIP_C_CRITICAL | FIP_C_FATAL, "Missing kernel package." },
    /* ONS_E_ADDRINUSE */       { FIP_C_ADDRESS, "Address already in use." },
    /* ONS_E_ADDRNOTAVAIL */    { FIP_C_ADDRESS, "The address is not available on this machine." },
    /* ONS_E_INVALADDR */       { FIP_C_ADDRESS | FIP_C_CRITICAL | FIP_C_FATAL, "Invalid/Unknown address." },
    /* ONS_E_DESTREQ */         { FIP_C_ADDRESS | FIP_C_CRITICAL | FIP_C_FATAL, "Destination address required." },
    /* ONS_E_NETDOWN */         { FIP_C_CONNECTING, "Network is down." },
    /* ONS_E_NETUNREACH */      { FIP_C_CONNECTING, "Network unreachable." },
    /* ONS_E_HOSTDOWN */        { FIP_C_CONNECTING, "Host is down." },
    /* ONS_E_HOSTUNREACH */     { FIP_C_CONNECTING, "Host is unreachable." },
    /* ONS_E_ISCONN */          { FIP_C_CONNECTING, "Already connected." },
    /* ONS_E_REFUSED */         { FIP_C_CONNECTING, "Connection refused by other side." },
    /* ONS_E_TIMEDOUT */        { FIP_C_CONNECTING, "Operation timed out." },
    /* ONS_E_NETRESET */        { FIP_C_SHUTDOWN | FIP_C_CLOSED, "Connection closed due to network reset." },
    /* ONS_E_RESET */           { FIP_C_SHUTDOWN | FIP_C_CLOSED, "Connection reset by peer." },
    /* ONS_E_CONNABORT */       { FIP_C_SHUTDOWN | FIP_C_CLOSED, "The connection has been aborted by our side." },
    /* ONS_E_SHUTDOWN */        { FIP_C_IO | FIP_C_CRITICAL | FIP_C_FATAL, "Connection already shutdown." },
    /* ONS_E_PIPE */            { FIP_C_IO | FIP_C_CLOSED, "Local end has already been closed." },
    /* ONS_E_NOTCONN */         { FIP_C_IO, "Socket is not connected, yet." },
    /* ONS_E_MSGSIZE */         { FIP_C_IO | FIP_C_CRITICAL | FIP_C_FATAL, "Message size too big." },
    /* ONS_E_DROP */            { FIP_C_IO | FIP_C_CRITICAL | FIP_C_TEMPORARY, "Packet dropped due to queue overflow (most time packages are dropped silently)." },
    /* ONS_E_MFILE */           { FIP_C_BINDING | FIP_C_CRITICAL | FIP_C_FATAL, "Process file descriptor table overflow." },
    /* ONS_E_NFILE */           { FIP_C_BINDING | FIP_C_CRITICAL | FIP_C_FATAL, "Global file descriptor table overflow." },
    /* ONS_E_PROVERFLOW */      { FIP_C_BINDING | FIP_C_CRITICAL | FIP_C_TEMPORARY, "Port number or routing cache overflow." },
    /* ONS_E_PROTOFAIL */       { FIP_C_BINDING | FIP_C_CRITICAL | FIP_C_TEMPORARY, "Protocol error." },
    /* ONS_E_INVALIDDEV */      { FIP_C_BINDING | FIP_C_CRITICAL | FIP_C_FATAL, "Invalid device." },
    /* ONS_E_NOPROTOOPT */      { FIP_C_OPTION | FIP_C_CRITICAL | FIP_C_FATAL, "The option is unknown at the level indicated." },
    /* ONS_E_ISDIR */           { FIP_C_FILE, "Is a directory." },
    /* ONS_E_TOOLARGE */        { FIP_C_FILE, "File is too large." },
    /* ONS_E_NOSPACELEFT */     { FIP_C_FILE, "No space left on device." },
    /* ONS_E_QUOTA */           { FIP_C_FILE, "Quota exceeded." },
    /* ONS_E_LOOP */            { FIP_C_FILE, "Too many symlinks encountered in local address." },
    /* ONS_E_NAMETOOLONG */     { FIP_C_FILE, "The local address has an overlength." },
    /* ONS_E_NOFILE */          { FIP_C_FILE, "File does not exist." },
    /* ONS_E_NOTDIR */          { FIP_C_FILE, "Invalid directory path." },
    /* ONS_E_ROFS */            { FIP_C_FILE | FIP_C_CRITICAL | FIP_C_FATAL, "Read only filesystem." },
    /* ONS_E_IO */              { FIP_C_FILE | FIP_C_CRITICAL | FIP_C_TEMPORARY, "IO failure." }
};

/* Takes an internal system dependent error code and returns the
 * FIP equivalent.
 */
/** Linux error codes:
 * Error codes mentioned in linux socket man pages are marked with a "*".
 * Codes which are mentioned somewhere else in the endless google result lists about
 * sockets are marked with a "^".
 * Codes which initiate a socket close are marked with a "#".
 * Codes with an undefined behaviour (in socket functions) are marked with a "?".
 * Codes which occur with file sockets are marked with a "+".
 */
ons_err_t fip_mkerrno(signed int err) {
    switch(err) {
    /* general error codes */
        case EPERM: /* (*^+) Operation not permitted */
        case EACCES: /* (*^+) Permission denied */
            return ONS_E_DENIED;
        case EINTR: /* (*^+) Interrupted system call */
            return ONS_E_SIGFLOOD;
        case EBADF: /* (*^#+) Bad file number */
        case ENOTSOCK: /* (*^+) Socket operation on non-socket */
            return ONS_E_BADFD;
        case ENOMEM: /* (*^+) Out of memory */
        case ENOBUFS: /* (*^+) No buffer space available */
            return ONS_E_MEMFAIL;
        case EALREADY: /* (*^+) Operation already in progress */
            return ONS_E_ALREADY;
        case EINPROGRESS: /* (*^+) Operation now in progress */
            return ONS_E_NONBLOCK;
        case EFAULT: /* (*^+) Bad address */
            return ONS_E_BADARG;
        case EINVAL: /* (*^+) Invalid argument */
            return ONS_E_BADARG;
        case EISCONN: /* (*^+) Transport endpoint is already connected */
            return ONS_E_ISCONN;
        case ENOTCONN: /* (*^+) Transport endpoint is not connected */
            return ONS_E_NOTCONN;
    /* fd creation failed */
        case ENFILE: /* (*^+) File table overflow */
            return ONS_E_NFILE;
        case EMFILE: /* (*^+) Too many open files */
            return ONS_E_MFILE;
    /* remote host errors */
        case EHOSTDOWN: /* (*^+) Host is down */
            return ONS_E_HOSTDOWN;
        case EHOSTUNREACH: /* (*^+) No route to host */
            return ONS_E_HOSTUNREACH;
        case ETIMEDOUT: /* (*^+) Connection timed out */
            return ONS_E_TIMEDOUT;
        case ECONNREFUSED: /* (*^+) Connection refused */
            return ONS_E_REFUSED;
        case ENETDOWN: /* (*^+) Network is down */
            return ONS_E_NETDOWN;
        case ENETUNREACH: /* (*^+) Network is unreachable */
            return ONS_E_NETUNREACH;
        case ENETRESET: /* (*^+#) Network dropped connection because of reset */
            return ONS_E_NETRESET;
        case ECONNABORTED: /* (*^+#) Software caused connection abort */
            return ONS_E_CONNABORT;
        case ECONNRESET: /* (*^+#) Connection reset by peer */
            return ONS_E_RESET;
    /* not supported/available */
        case ENOPROTOOPT: /* (*^+) Protocol not available */
            return ONS_E_NOPROTOOPT;
        case EPROTONOSUPPORT: /* (*^+) Protocol not supported */
        case ESOCKTNOSUPPORT: /* (*^+) Socket type not supported */
        case EPROTOTYPE: /* (*^) Protocol wrong type for socket */
            return ONS_E_PROTODOM;
        case EOPNOTSUPP: /* (*^+) Operation not supported on transport endpoint */
            return ONS_E_OPNOTSUPP;
        case EPFNOSUPPORT: /* (*^+) Protocol family not supported */
            return ONS_E_PFNOTSUPP;
        case EAFNOSUPPORT: /* (*^+) Address family not supported by protocol */
            return ONS_E_AFNOTSUPP;
        case ENOPKG: /* (*^+?) Package not installed */
            return ONS_E_MISSING;
    /* address/device failure */
        case ENXIO: /* (*^) No such device or address. */
        case ENODEV: /* (^) No such device */
            return ONS_E_INVALIDDEV;
        case EDESTADDRREQ: /* (*^+) Destination address required */
            return ONS_E_DESTREQ;
        case EADDRINUSE: /* (*^+) Address already in use */
            return ONS_E_ADDRINUSE;
        case EADDRNOTAVAIL: /* (*^+) Cannot assign requested address */
            return ONS_E_ADDRNOTAVAIL;
    /* IO failure */
        case EPIPE: /* (*^#+) Broken pipe */
            return ONS_E_PIPE;
        case EMSGSIZE: /* (*^+) Message too long */
            return ONS_E_MSGSIZE;
        case ESHUTDOWN: /* (*^+) Cannot send after transport endpoint shutdown */
            return ONS_E_SHUTDOWN;
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
        case EWOULDBLOCK: /* (*^+) Operation would block */
#endif
        case EAGAIN: /* (*^+) Try again */
            return ONS_E_AGAIN;
        case EPROTO: /* (*^+?) Protocol error */
            return ONS_E_PROTOFAIL;
        default:
            return fip_mkerrno_file(err);
    }
}

/* Outsourcing of fip_mkerrno with all unix socket related
 * error codes.
 */
ons_err_t fip_mkerrno_file(signed int err) {
    switch(err) {
    /* general errors */
        case EIO: /* (*^?+) I/O error */
            return ONS_E_IO;
    /* file/path found/not found */
        case ENOENT: /* (*^+) No such file or directory */
            return ONS_E_NOFILE;
        case ENOTDIR: /* (+) Not a directory */
            return ONS_E_NOTDIR;
        case EISDIR: /* (+) Is a directory */
            return ONS_E_ISDIR;
        case EMLINK: /* (+) Too many links */
        case ELOOP: /* (+) Too many symbolic links encountered */
            return ONS_E_LOOP;
        case ENAMETOOLONG: /* (+) File name too long */
            return ONS_E_NAMETOOLONG;
    /* writing */
        case EFBIG: /* (+) File too large */
            return ONS_E_TOOLARGE;
        case ENOSPC: /* (+) No space left on device */
            return ONS_E_NOSPACELEFT;
        case EROFS: /* (+) Read-only file system */
            return ONS_E_ROFS;
        case EDQUOT: /* (+) Quota exceeded */
            return ONS_E_QUOTA;
        default:
            return ONS_E_FAIL;
    }
}

