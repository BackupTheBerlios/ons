/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 3. June 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Socket functions.
 * This file provides a socket interface similar to the Berkeley Sockets.
 */

#define ONS_CONF_ECODES
#include "fip/fip.h"
#include <saw/saw.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef ONS_CONF_HAVE_SYS_TYPES_H
    #include <sys/types.h>
#endif
#ifdef ONS_CONF_HAVE_UNISTD_H
    #include <unistd.h>
#endif
#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif
#ifdef ONS_CONF_HAVE_NETINET_IN_H
    #include <netinet/in.h>
#endif
#ifdef ONS_CONF_HAVE_FCNTL_H
    #include <fcntl.h>
#endif

/* Creates a new socket.
 * This function creates a new socket and returns the related identifier. On failure
 * FIP_INVALID_SOCKET is returned and \err is set respectively.
 *
 * Parameters:
 * \err has to be a pointer to a variable where to store the error value.
 * \dom specifies the domain of the socket (eg., SAW_ADDR4_DOM, SAW_ADDR6_DOM).
 * \trans specifies the transmission method (eg., FIP_TRANS_DGRAM).
 * \proto sepcifies the protocol (eg., FIP_PROTO_UDP).
 *
 * Return value:
 * This function returns FIP_INVALID_SOCKET on failure or returns the socket id on success.
 *
 * Error codes:
 * FIP_E_PFNOTSUPP: The protocol family used in \dom is not supported on your machine.
 * FIP_E_PROTONOTSUPP: The protocol (\proto) or transmission method (\trans) is not known or not available on your machine.
 * FIP_E_PROTODOM: The protocol (\proto) is not supported/valid in \trans, or \trans is not supported/valid in \dom.
 * FIP_E_DENIED: You have no permission to create a socket of the specified type.
 * FIP_E_MFILE: The process' limit on the total number of open files has been reached.
 * FIP_E_NFILE: The system's limit on the total number of open files has been reached.
 * FIP_E_MEMFAIL: The process can't get enough memory to open this socket.
 * FIP_E_FAIL: An unknown error occurred.
 */
fip_socket_t fip_socket(fip_err_t *err, signed int dom, unsigned int trans, unsigned int proto) {
    fip_socket_t fd;
#ifdef ONS_CONF_WINDOWS
    if((fd = socket(FIP_MKDOM(dom), FIP_MKTRANS(trans), FIP_MKPROTO(proto))) == INVALID_SOCKET) {
#else
    if((fd = socket(FIP_MKDOM(dom), FIP_MKTRANS(trans), FIP_MKPROTO(proto))) == -1) {
#endif
        switch(errno) {
            case EAFNOSUPPORT:      /* The implementation does not support the specified address family. */
            case EPFNOSUPPORT:      /* We handle protocol and address families the same way here. */
                *err = FIP_E_PFNOTSUPP;
                return FIP_INVALID_SOCKET;
            case EINVAL:            /* Unknown protocol, or protocol family not available. */
                *err = FIP_E_PROTONOTSUPP;
                return FIP_INVALID_SOCKET;
            case ESOCKTNOSUPPORT:   /* Socket type (transmission method) not supported */
            case EPROTONOSUPPORT:   /* The protocol type or the specified protocol is not supported within this domain. */
                *err = FIP_E_PROTODOM;
                return FIP_INVALID_SOCKET;
            case EPERM:
            case EACCES:           /* Permission to create a socket of the specified type and/or protocol is denied. */
                *err = FIP_E_DENIED;
                return FIP_INVALID_SOCKET;
            case EMFILE:            /* Process' file table overflow. */
                *err = FIP_E_MFILE;
                return FIP_INVALID_SOCKET;
            case ENFILE:            /* The system's limit on the total number of open files has been reached. */
                *err = FIP_E_NFILE;
                return FIP_INVALID_SOCKET;
            case ENOBUFS:
            case ENOMEM:            /* Insufficient memory is available. */
                *err = FIP_E_MEMFAIL;
                return FIP_INVALID_SOCKET;
            default:
                *err = FIP_E_FAIL;
                return FIP_INVALID_SOCKET;
        }
    }
    return fd;
}

/* Creates a new pair of sockets.
 * Both sockets are connected to each other and are of protocol family SAW_ADDRLO_DOM.
 * Usually this is used before a fork() or thread start and the sockets are used for IPC. The communication file
 * is in an abstract namespace and cannot be accessed by other processes.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \trans is the transmission method which should be used.
 * \proto is the protocol which should be used.
 * \fd1 is a pointer to an uninitialized fd where the first fd is stored.
 * \fd2 same as \fd1 but the second fd is stored here.
 *
 * Return value:
 * \fd1 and \fd2 contain both file descriptors on success and 1 is returned.
 * On failure 0 is returned and \err is set respectively. \fd1 and \fd2 are not changed.
 *
 * Error codes:
 * FIP_E_PFNOTSUPP: Your socket implementation does not support unix socketpairs.
 * FIP_E_PROTONOTSUPP: The protocol (\proto) is not known or not available on your machine.
 * FIP_E_PROTODOM: The protocol (\proto) is not supported/valid in \trans, or \trans is not supported/valid in Unix sockets.
 * FIP_E_DENIED: You have no permission to create a socket of the specified type.
 * FIP_E_MFILE: The process' limit on the total number of open files has been reached.
 * FIP_E_NFILE: The system's limit on the total number of open files has been reached.
 * FIP_E_MEMFAIL: The process can't get enough memory to open this socket.
 * FIP_E_OPNOTSUPP: Operation not supported on this system.
 * FIP_E_FAIL: An unknown error occurred.
 */
bool fip_socketpair(fip_err_t *err, unsigned int trans, unsigned int proto, fip_socket_t *fd1, fip_socket_t *fd2) {
#ifdef ONS_CONF_HAVE_UNIX_SOCKETS
    fip_socket_t tfd[2];

    if(socketpair(FIP_MKDOM(SAW_ADDRLO_DOM), FIP_MKTRANS(trans), FIP_MKPROTO(proto), tfd) != 0) {
        switch(errno) {
            case EAFNOSUPPORT:      /* The implementation does not support the specified address family. */
            case EPFNOSUPPORT:      /* PF/AF are handled the same way here. */
                *err = FIP_E_PFNOTSUPP;
                return 0;
            case EPROTONOSUPPORT:   /* The protocol type or the specified protocol is not supported within this domain. */
                *err = FIP_E_PROTODOM;
                return 0;
            case EOPNOTSUPP:        /* The specified protocol does not support creation of socket pairs. */
            case ESOCKTNOSUPPORT:   /* Socket type (transmission method) not supported */
            case EINVAL:            /* Unknown protocol, or protocol family not available. */
                *err = FIP_E_PROTONOTSUPP;
                return 0;
            case EMFILE:            /* Process file table overflow. */
                *err = FIP_E_MFILE;
                return 0;
            case ENFILE:            /* The system limit on the total number of open files has been reached. */
                *err = FIP_E_NFILE;
                return 0;
            case ENOBUFS:
            case ENOMEM:            /* Insufficient memory is available. */
                *err = FIP_E_MEMFAIL;
                return 0;
            case EPERM:
            case EACCES:           /* Permission to create a socket of the specified type and/or protocol is denied. */
                *err = FIP_E_DENIED;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    *fd1 = tfd[0];
    *fd2 = tfd[1];
    return true;
#else
    *err = FIP_E_OPNOTSUPP;
    return false;
#endif
}

/* Binds a socket.
 * This functions binds a specific address to a socket. If you want to bind to an interface, please look
 * into if.h/c.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket file descriptor which must be valid (eg., created with fip_socket()).
 * \addr is a Libsaw address which is bound to the socket.
 *
 * Return value:
 * Returns 1 on success and 0 on failure. \err is set respectively.
 *
 * Error codes:
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_DENIED: The address is protected and you do not have the privileges to bind to this address.
 * FIP_E_ADDRINUSE: The address is already in use.
 * FIP_E_BADARG: \fd has already an address bound or you passed an invalid address.
 * FIP_E_ADDRNOTAVAIL: This address is not available on this machine.
 * FIP_E_FAIL: Unknown failure.
 *
 * The following error codes occur only on unix sockets:
 * FIP_E_ISDIR: Is a directory.
 * FIP_E_NOTDIR: Invalid directory path.
 * FIP_E_NOSPACELEFT: No space left on device.
 * FIP_E_QUOTA: Quota exceeded.
 * FIP_E_LOOP: Too many symlinks encountered in local address.
 * FIP_E_NAMETOOLONG: The local address has an overlength.
 * FIP_E_ROFS: Read only filesystem.
 * FIP_E_IO: IO failure.
 */
bool fip_bind(fip_err_t *err, fip_socket_t fd, const saw_addr_t *addr) {
    uint8_t addr_buffer[SAW_ADDR_LOCSIZE];
    saw_addr_local_t *saddr = (saw_addr_local_t*)addr_buffer;

    saw_addr_to_local(addr, saddr);

    if(bind(fd, saddr, saw_addr_locsize(saddr)) == -1) {
        switch(errno) {
            case EBADF: /* \fd is not a valid descriptor. */
            case ENOTSOCK: /* \fd is a descriptor for a file, not a socket. */
                *err = FIP_E_BADFD;
                return 0;
            case EPERM:
            case EACCES: /* The address is protected, and the user is not the superuser. */
                *err = FIP_E_DENIED;
                return 0;
            case EADDRINUSE: /* The given address is already in use. */
                *err = FIP_E_ADDRINUSE;
                return 0;
            case EINVAL: /* The socket is already bound to an address or invalid argument. */
                *err = FIP_E_BADARG;
                return 0;
            case EADDRNOTAVAIL:
                *err = FIP_E_ADDRNOTAVAIL;
                return 0;
            case EIO: /* File: I/O error */
                *err = FIP_E_IO;
                return 0;
            case ENOTDIR: /* File: Not a directory */
                *err = FIP_E_NOTDIR;
                return 0;
            case EISDIR: /* File: Is a directory */
                *err = FIP_E_ISDIR;
                return 0;
            case ENOSPC: /* File: No space left on device */
                *err = FIP_E_NOSPACELEFT;
                return 0;
            case EROFS: /* File: Read-only file system */
                *err = FIP_E_ROFS;
                return 0;
            case EMLINK: /* File: Too many links */
            case ELOOP: /* File: Too many symbolic links encountered */
                *err = FIP_E_LOOP;
                return 0;
            case ENAMETOOLONG: /* File: File name too long */
                *err = FIP_E_NAMETOOLONG;
                return 0;
            case EDQUOT: /* File: Quota exceeded */
                *err = FIP_E_QUOTA;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
}

/* Creates a listener on a socket.
 * Sets a socket into listening state. Now, the socket can accept new connections through the fip_accept() call.
 * The socket cannot be used to perform other operations than accepting.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket which must be bound.
 * \backlog specifies the size of the listening queue. This amount of connections can be queued until
 *          new connections are rejected. It is cropped to INT_MAX if it is bigger. Though, most systems
 *          specify a maximum of ~100 and autocrop the value.
 *
 * Return value:
 * Returns 1 on success and 0 on failure.
 *
 * Error codes:
 * FIP_E_ADDRINUSE: Address is already in use.
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_BADARG: \fd is not a bound unconnected listenable socket.
 * FIP_E_FAIL: Invalid error occurred.
 */
bool fip_listen(fip_err_t *err, fip_socket_t fd, unsigned int backlog) {
    /* truncate backlog (unix defines a signed integer here!) */
    if(backlog > INT_MAX) backlog = INT_MAX;
    if(listen(fd, (signed int)backlog) == -1) {
        switch(errno) {
            case EADDRINUSE: /* Another socket is already listening on the same port. */
                *err = FIP_E_ADDRINUSE;
                return 0;
            case EBADF: /* The argument sockfd is not a valid descriptor. */
            case ENOTSOCK: /* The argument sockfd is not a socket. */
                *err = FIP_E_BADFD;
                return 0;
            case EINVAL:
            case EFAULT:
            case EISCONN:
            case EOPNOTSUPP: /* The socket is not of a type that supports the listen() operation. */
                *err = FIP_E_BADARG;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
}

/* Connects to a remote socket.
 * Connects a local socket to a remote address. If the socket is not bound yet it is bound to a
 * local address.
 * If you connect connection-less sockets the remote address is taken as default destination.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket file descriptor which must be valid (eg., created with fip_socket()).
 * \addr is the remote address where the socket connects to.
 *
 * Return value:
 * On failure 0 is returned and \err is set respectively. On success, 1 is returned and
 * \err is set to FIP_E_NONE or to FIP_E_NONBLOCK if the connection is continued in background.
 *
 * Error codes:
 * FIP_E_DENIED: The user tried to connect to a broadcast address without having
 *               the socket broadcast flag enabled or the connection request
 *               failed because of a local firewall rule.
 *               On Unix Sockets it can indicate a read-only FS or directory.
 * FIP_E_SIGFLOOD: Signal flood.
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_HOSTDOWN: Remote host is down.
 * FIP_E_HOSTUNREACH: Remote host is unreachable.
 * FIP_E_AFNOTSUPP: The address is not of a valid family for the socket type.
 * FIP_E_INVALADDR: Invalid address given.
 * FIP_E_NETDOWN: Network is down.
 * FIP_E_NETUNREACH: Network is unreachable.
 * FIP_E_ISCONN: \fd is already connected.
 * FIP_E_TIMEDOUT: Connection attempt timed out.
 * FIP_E_REFUSED: Other side is not listening.
 * FIP_E_ALREADY: Another connection attempt is already in progress on \fd.
 * FIP_E_ADDRINUSE: The local address is already in use.
 * FIP_E_PROVERFLOW: \fd is not bound and there are no free local ports, anymore.
 *
 * Unix socket errors:
 * FIP_E_NOFILE: Remote address does not exist.
 * FIP_E_NOTDIR: Invalid path.
 * FIP_E_ISDIR: Destination is a directory.
 * FIP_E_LOOP: Too many loops in path.
 * FIP_E_ROFS: Readonly filesystem.
 */
bool fip_connect(fip_err_t *err, fip_socket_t fd, const saw_addr_t *addr) {
    char addr_buffer[SAW_ADDR_LOCSIZE];
    saw_addr_local_t *saddr = (saw_addr_local_t*)addr_buffer;
    unsigned int eintr_count = 0;

    saw_addr_to_local(addr, saddr);

    *err = FIP_E_NONE;
    eintr_again:
    if(connect(fd, saddr, saw_addr_locsize(saddr)) == -1) {
        switch(errno) {
            case EPERM:
            case EACCES: /* The user tried to connect to a broadcast address without having
                            the socket broadcast flag enabled or the connection request
                            failed because of a local firewall rule.
                            On Unix Sockets it can indicate a read-only FS or directory. */
                *err = FIP_E_DENIED;
                return 0;
            case EINTR: /* The system call was interrupted by a signal that was caught. */
                if(fip_eintr_count == FIP_EINTR_ENDLESS) goto eintr_again;
                else if(++eintr_count >= fip_eintr_count) {
                    *err = FIP_E_SIGFLOOD;
                    return 0;
                }
                else goto eintr_again;
            case ENOTSOCK: /* The file descriptor is not associated with a socket. */
            case EBADF: /* The file descriptor is not a valid index in the descriptor table. */
                *err = FIP_E_BADFD;
                return 0;
            case EHOSTDOWN: /* Host is down. */
                *err = FIP_E_HOSTDOWN;
                return 0;
            case EHOSTUNREACH: /* No route to host. */
                *err = FIP_E_HOSTUNREACH;
                return 0;
            case EAFNOSUPPORT: /* The passed address didn't have the correct address family in its sa_family field. */
                *err = FIP_E_AFNOTSUPP;
                return 0;
            case EINVAL:
            case EFAULT: /* Invalid addess. */
                *err = FIP_E_INVALADDR;
                return 0;
            case ENETDOWN: /* Network is down. */
                *err = FIP_E_NETDOWN;
                return 0;
            case ENETUNREACH: /* Network unreachable. */
                *err = FIP_E_NETUNREACH;
                return 0;
            case EISCONN: /* Already connected. */
                *err = FIP_E_ISCONN;
                return 0;
            case ETIMEDOUT: /* Operation timed out. */
                *err = FIP_E_TIMEDOUT;
                return 0;
            case ECONNREFUSED: /* Other side refused connection. */
                *err = FIP_E_REFUSED;
                return 0;
            case EALREADY: /* Other connection is already in progress. */
                *err = FIP_E_ALREADY;
                return 0;
            case EINPROGRESS: /* Operation continued in background. */
                *err = FIP_E_NONBLOCK;
                break;
            case EADDRINUSE: /* Address already in use. */
                *err = FIP_E_ADDRINUSE;
                return 0;
            case EAGAIN: /* no more free local ports */
                *err = FIP_E_PROVERFLOW;
                return 0;
            case ENOENT: /* File: No such file or directory */
            case ENAMETOOLONG: /* File: File name too long */
                *err = FIP_E_NOFILE;
                return 0;
            case ENOTDIR: /* File: Not a directory */
                *err = FIP_E_NOTDIR;
                return 0;
            case EISDIR: /* File: Is a directory */
                *err = FIP_E_ISDIR;
                return 0;
            case EMLINK: /* File: Too many links */
            case ELOOP: /* File: Too many symbolic links encountered */
                *err = FIP_E_LOOP;
                return 0;
            case EROFS: /* File: read only filesystem */
                *err = FIP_E_ROFS;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
}

/* Accepts a connection on a listener.
 * Accepts a single pending connection from the listener queue and returns a new created socket
 * which represents the new connection. You can use the new socket as if you connected it
 * yourself to the peer.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket which must be in listening state.
 * \addr: if \addr is not NULL the peer's address is stored here.
 *
 * Return value:
 * Returns a newly created socket on success and \err set to FIP_E_NONE, otherwise FIP_INVALID_SOCKET
 * and \err is filled with the error code.
 *
 * Error codes:
 * FIP_E_NONBLOCK: The operation would block.
 * FIP_E_SIGFLOOD: The operation got sigflooded.
 * FIP_E_BADARG: Socket is not a connection-oriented listener.
 * FIP_E_MFILE: The system limit on the total number of open files has been reached.
 * FIP_E_NFILE: The process limit on the total number of open files has been reached.
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_MEMFAIL: No more socket buffer memory available.
 * FIP_E_PROTOFAIL: Protocol failure.
 * FIP_E_CONNABORT: The listener socket was closed.
 * FIP_E_DENIED: Firewall forbids new connections.
 * FIP_E_INVALADDR: The address format of the socket is not known. This does not cancel the operation.
 * FIP_E_FAIL: Invalid error occurred.
 */
fip_socket_t fip_accept(fip_err_t *err, fip_socket_t fd, saw_addr_t *addr) {
    char addr_buffer[SAW_ADDR_LOCSIZE];
    saw_addr_local_t *saddr = (saw_addr_local_t*)addr_buffer;
    unsigned int addrsize = SAW_ADDR_LOCSIZE, eintr_count = 0;
    fip_socket_t ret;

    *err = FIP_E_NONE;

    eintr_again:
    ret = addr?accept(fd, saddr, &addrsize):accept(fd, NULL, NULL);
    if(ret == -1) {
        switch(errno) {
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            case EWOULDBLOCK:
#endif
            case EAGAIN: /* The socket is marked non-blocking and no connections are present
                            to be accepted. */
                *err = FIP_E_NONBLOCK;
                return FIP_INVALID_SOCKET;
            case EINTR: /* The system call was interrupted by a signal that was caught
                           before a valid connection arrived. */
                if(fip_eintr_count == FIP_EINTR_ENDLESS) goto eintr_again;
                else if(++eintr_count >= fip_eintr_count) {
                    *err = FIP_E_SIGFLOOD;
                    return FIP_INVALID_SOCKET;
                }
                else goto eintr_again;
            case EOPNOTSUPP: /* The referenced socket is not of type SOCK_STREAM. */
            case EFAULT: /* The addr argument is not in a writable part of the user address space. */
            case EINVAL: /* Socket is not listening for connections, or addrlen is invalid
                            (e.g., is negative). */
                *err = FIP_E_BADARG;
                return FIP_INVALID_SOCKET;
            case EMFILE: /* The per-process limit of open file descriptors has been reached. */
                *err = FIP_E_MFILE;
                return FIP_INVALID_SOCKET;
            case ENFILE: /* The  system  limit  on  the  total number of open files has been reached. */
                *err = FIP_E_NFILE;
                return FIP_INVALID_SOCKET;
            case ENOTSOCK: /* The descriptor references a file, not a socket. */
            case EBADF: /* The descriptor is invalid. */
                *err = FIP_E_BADFD;
                return FIP_INVALID_SOCKET;
            case ENOBUFS:
            case ENOMEM: /* Not enough free memory. This often means that the memory allocation
                            is limited by the socket buffer limits, not by the system memory. */
                *err = FIP_E_MEMFAIL;
                return FIP_INVALID_SOCKET;
            case EPROTO: /* Protocol error. */
                *err = FIP_E_PROTOFAIL;
                return FIP_INVALID_SOCKET;
            case ECONNABORTED: /* A connection has been aborted. */
                *err = FIP_E_CONNABORT;
                return FIP_INVALID_SOCKET;
            case EPERM: /* Firewall rules forbid connection. */
                *err = FIP_E_DENIED;
                return FIP_INVALID_SOCKET;
            default:
                /* In addition, network errors for the new socket and as defined for the
                 * protocol may be returned.  Various Linux kernels can return other
                 * errors such as ENOSR, ESOCKTNOSUPPORT, EPROTONOSUPPORT, ETIMEDOUT.
                 */
                *err = FIP_E_FAIL;
                return FIP_INVALID_SOCKET;
        }
    }
    if(addr) {
        if(!saw_addr_from_local(saddr, addr)) *err = FIP_E_INVALADDR; /* Set an addr-failure but return new fd. */
    }
    return ret;
}

/* Writes data on a socket.
 * Sends the given amount of data to a remote socket. Several options can be set to control the
 * delivery of the data. If the socket is in nonblocking mode there is a maximum amount of data
 * which can be delivered in one syscall, see return values for more infos.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a file descriptor which must be binded before.
 * \buf is the buffer of data which is sent through the socket.
 * \bufsize is the size of \buf.
 * \addr is the destination. If the socket is a connection-oriented socket, this must be NULL. An
 *       connection-less socket must have \addr set, unless it has a default destination set.
 * \opts specifies some delivery options. Only FIP_TOPT_* are allowed here.
 *
 * Return value:
 * This function returns the number of bytes sent through the socket. If it is less than \bufsize
 * you have to deliver the rest of \buf in the next call. This technique prevents blocking send()
 * calls.
 * If the return code is zero, nothing is transmitted and \err is set.
 * On success \err is always set to FIP_E_NONE;
 *
 * Error codes:
 * FIP_E_BADARG: Some flag in the \opts parameter is invalid or
 *               \buf/\addr points into invalid memory space or
 *               \bufsize is 0 or \addr is of invalid address family.
 * FIP_E_BADFD: \fd is not a socket.
 * FIP_E_NONBLOCK: The operation would block.
 * FIP_E_DROP: The package was dropped due to queue overflow.
 * FIP_E_MEMFAIL: No memory available.
 * FIP_E_SIGFLOOD: The operation got sigflooded.
 * FIP_E_MSGSIZE: The size of the buffer is greater than the MTU.
 * FIP_E_DESTREQ: Destination address required.
 * FIP_E_ISCONN: The socket is connected but a recipient was specified.
 * FIP_E_NOTCONN: Socket is not connected and no target has been given.
 * FIP_E_PIPE: The socket was already closed before (some systems send signals here!).
 * FIP_E_RESET: Connection reset by peer.
 * FIP_E_HOSTDOWN: Remote host is down.
 * FIP_E_HOSTUNREACH: Remote host is unreachable.
 * FIP_E_NETDOWN: Network is down.
 * FIP_E_NETUNREACH: Network is unreachable.
 * FIP_E_CONNABORT: Software caused connection abort.
 * FIP_E_NETRESET: Connection lost due network reset.
 * FIP_E_SHUTDOWN: Local sending end-point has already been shutdown.
 * FIP_E_REFUSED: Other side refused connection.
 * FIP_E_FAIL: Invalid error occurred.
 *
 * Unix socket related errors:
 * FIP_E_DENIED: (only unix socket) Write access to file is denied.
 * FIP_E_IO: File IO error.
 * FIP_E_NOFILE: No such file,
 * FIP_E_NOTDIR: Invalid path.
 * FIP_E_ISDIR: Destination is a directory.
 * FIP_E_LOOP: Too many symbolic links.
 * FIP_E_TOOLARGE: File is too large.
 * FIP_E_NOSPACELEFT: No space left on device.
 * FIP_E_QUOTA: Quota exceeded.
 */
unsigned int fip_send(fip_err_t *err, fip_socket_t fd, const char *buf, unsigned int bufsize, const saw_addr_t *addr, unsigned int opts) {
    char addr_buffer[SAW_ADDR_LOCSIZE];
    saw_addr_local_t *saddr = (saw_addr_local_t*)addr_buffer;
    signed int ret;
    unsigned int eintr_count = 0;

    *err = FIP_E_NONE;
    if(bufsize == 0) {
        *err = FIP_E_BADARG;
        return 0;
    }
    if(addr) saw_addr_to_local(addr, saddr);

    eintr_again:
    if((ret = sendto(fd, buf, bufsize, FIP_MKTOPTS(opts), addr?saddr:NULL, addr?saw_addr_locsize(saddr):0)) == -1) {
        switch(errno) {
            case EINVAL: /* Invalid argument passed. */
            case EFAULT: /* An invalid user space address was specified for a parameter. */
            case EAFNOSUPPORT: /* (*^+) Address family not supported by protocol */
            case EOPNOTSUPP: /* Some bit in the flags argument is inappropriate for the socket type. */
                *err = FIP_E_BADARG;
                return 0;
            case ENOTSOCK: /* The argument \fd is not a socket. */
            case EBADF: /* An invalid descriptor was specified. */
                *err = FIP_E_BADFD;
                return 0;
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            case EWOULDBLOCK:
#endif
            case EAGAIN: /* The socket is marked non-blocking and the requested operation would block. */
                *err = FIP_E_NONBLOCK;
                return 0;
            case ENOBUFS: /* The output queue for a network interface was full. This gener-
                             ally indicates that the interface has stopped sending, but may
                             be caused by transient congestion. (Normally, this does not
                             occur in Linux. Packets are just silently dropped when a device
                             queue overflows.) */
                *err = FIP_E_DROP;
                return 0;
            case ENOMEM: /* No memory available. */
                *err = FIP_E_MEMFAIL;
                return 0;
            case EINTR: /* A signal occurred before any data was transmitted. */
                if(fip_eintr_count == FIP_EINTR_ENDLESS) goto eintr_again;
                else if(++eintr_count >= fip_eintr_count) {
                    *err = FIP_E_SIGFLOOD;
                    return 0;
                }
                else goto eintr_again;
            case EMSGSIZE: /* The socket type requires that message be sent atomically, and
                              the size of the message to be sent made this impossible. */
                *err = FIP_E_MSGSIZE;
                return 0;
            case EDESTADDRREQ: /* The socket is not connection-mode, and no peer address is set. */
                *err = FIP_E_DESTREQ;
                return 0;
            case ECONNREFUSED: /* Other side refused the network connection. */
                *err = FIP_E_REFUSED;
                return 0;
            case EISCONN: /* The connection-mode socket was connected already but a recipient
                             was specified. (Now either this error is returned, or the
                             recipient specification is ignored.) */
                *err = FIP_E_ISCONN;
                return 0;
            case ENOTCONN: /* The socket is not connected, and no target has been given. */
                *err = FIP_E_NOTCONN;
                return 0;
            case EPIPE: /* The local end has been shut down on a connection oriented
                           socket. In this case the process will also receive a SIGPIPE
                           unless MSG_NOSIGNAL is set. */
                *err = FIP_E_PIPE;
                return 0;
            case ECONNRESET: /* Connection reset by peer. */
                *err = FIP_E_RESET;
                return 0;
            case EHOSTDOWN: /* (*^+) Host is down */
                *err = FIP_E_HOSTDOWN;
                return 0;
            case EHOSTUNREACH: /* (*^+) No route to host */
                *err = FIP_E_HOSTUNREACH;
                return 0;
            case ENETDOWN: /* (*^+) Network is down */
                *err = FIP_E_NETDOWN;
                return 0;
            case ENETUNREACH: /* (*^+) Network is unreachable */
                *err = FIP_E_NETUNREACH;
                return 0;
            case ENETRESET: /* (*^+#) Network dropped connection because of reset */
                *err = FIP_E_NETRESET;
                return 0;
            case ECONNABORTED: /* (*^+#) Software caused connection abort */
                *err = FIP_E_CONNABORT;
                return 0;
            case ESHUTDOWN: /* (*^+) Cannot send after transport endpoint shutdown */
                *err = FIP_E_SHUTDOWN;
                return 0;
            case EPERM: /* (*^+) Operation not permitted */
            case EACCES: /* (*^+) Permission denied */
            case EROFS: /* (+) Read-only file system */
                *err = FIP_E_DENIED;
                return 0;
            case EIO: /* (*^?+) I/O error */
                *err = FIP_E_IO;
                return 0;
            case ENOENT: /* (*^+) No such file or directory */
            case ENAMETOOLONG: /* (+) File name too long */
                *err = FIP_E_NOFILE;
                return 0;
            case ENOTDIR: /* (+) Not a directory */
                *err = FIP_E_NOTDIR;
                return 0;
            case EISDIR: /* (+) Is a directory */
                *err = FIP_E_ISDIR;
                return 0;
            case EMLINK: /* (+) Too many links */
            case ELOOP: /* (+) Too many symbolic links encountered */
                *err = FIP_E_LOOP;
                return 0;
            case EFBIG: /* (+) File too large */
                *err = FIP_E_TOOLARGE;
                return 0;
            case ENOSPC: /* (+) No space left on device */
                *err = FIP_E_NOSPACELEFT;
                return 0;
            case EDQUOT: /* (+) Quota exceeded */
                *err = FIP_E_QUOTA;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return ret;
}

/* Reads data from a socket.
 * Reads a specific amount of data from a socket into a buffer. If \addr is given and \fd is a connection-less socket
 * the senders address is stored here. This functions blocks until data is available unless you
 * set the nonblocking option.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a valid socket which must already be binded.
 * \buf is the buffer where the result is stored.
 * \bufsize is the size of \buf.
 * \addr is the space where the result is stored. It must be big enough.
 * \opts specifies some delivery options. Only FIP_TOPT_* are allowed here.
 *
 * Return value:
 * This function returns the number of bytes read from the socket.
 * If the return code is zero nothing is read or an error occurred.
 * On success \err is always set to FIP_E_NONE, otherwise the error code is stored there.
 *
 * Error codes:
 * FIP_E_NONBLOCK: Operation would block or timeout expired.
 * FIP_E_BADARG: Either invalid socket passed or invalid transmission options.
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_MEMFAIL: Kernel buffers exceeded.
 * FIP_E_SIGFLOOD: Operation got sigflooded.
 * FIP_E_NOTCONN: \fd is of a connection-oriented protocol but not connected, yet.
 * FIP_E_RESET: Connection reset by peer.
 * FIP_E_NETRESET: Network reset.
 * FIP_E_CONNABORT: Connection has been aborted.
 * FIP_E_SHUTDOWN: Local receiving endpoint has already been shutdown.
 * FIP_E_DENIED: (unix socket) Read permission denied.
 * FIP_E_PIPE: Socket is already closed.
 * FIP_E_IO: (unix socket) IO error.
 * FIP_E_FAIL: Invalid error code occurred.
 */
unsigned int fip_recv(fip_err_t *err, fip_socket_t fd, char *buf, unsigned int size, saw_addr_t *addr, unsigned int opts) {
    char addr_buffer[SAW_ADDR_LOCSIZE];
    saw_addr_local_t *saddr = (saw_addr_local_t*)addr_buffer;
    signed int ret;
    unsigned int addrsize = SAW_ADDR_LOCSIZE;
    unsigned int eintr_count = 0;

    *err = FIP_E_NONE;
    if(!size) {
        *err = FIP_E_BADARG;
        return 0;
    }

    eintr_again:
    if((ret = recvfrom(fd, buf, size, FIP_MKTOPTS(opts), addr?saddr:NULL, addr?&addrsize:NULL)) == -1) {
        switch(errno) {
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            case EWOULDBLOCK:
#endif
            case EAGAIN: /* The socket is marked non-blocking and the receive operation
                            would block, or a receive timeout has been set and the timeout
                            expired before data was received. */
                *err = FIP_E_NONBLOCK;
                return 0;
            case EINVAL: /* Invalid argument passed. */
            case EFAULT: /* An invalid user space address was specified for a parameter. */
            case EOPNOTSUPP: /* Some bit in the flags argument is inappropriate for the socket type. */
                *err = FIP_E_BADARG;
                return 0;
            case ENOTSOCK: /* The argument \fd is not a socket. */
            case EBADF: /* An invalid descriptor was specified. */
                *err = FIP_E_BADFD;
                return 0;
            case ENOBUFS:
            case ENOMEM: /* No memory available. */
                *err = FIP_E_MEMFAIL;
                return 0;
            case EINTR: /* A signal occurred before any data was transmitted. */
                if(fip_eintr_count == FIP_EINTR_ENDLESS) goto eintr_again;
                else if(++eintr_count >= fip_eintr_count) {
                    *err = FIP_E_SIGFLOOD;
                    return 0;
                }
                else goto eintr_again;
            case ENOTCONN: /* The socket is not connected. */
                *err = FIP_E_NOTCONN;
                return 0;
            case EPIPE: /* The local end has been shut down on a connection oriented
                           socket. In this case the process will also receive a SIGPIPE
                           unless MSG_NOSIGNAL is set. */
                *err = FIP_E_PIPE;
                return 0;
            case ECONNRESET: /* Connection reset by peer. */
                *err = FIP_E_RESET;
                return 0;
            case ENETRESET: /* (*^+#) Network dropped connection because of reset */
                *err = FIP_E_NETRESET;
                return 0;
            case ECONNABORTED: /* (*^+#) Software caused connection abort */
                *err = FIP_E_CONNABORT;
                return 0;
            case ESHUTDOWN: /* (*^+) Cannot send after transport endpoint shutdown */
                *err = FIP_E_SHUTDOWN;
                return 0;
            case EPERM: /* (*^+) Operation not permitted */
            case EACCES: /* (*^+) Permission denied */
            case EROFS: /* (+) Read-only file system */
                *err = FIP_E_DENIED;
                return 0;
            case EIO: /* (*^?+) I/O error */
                *err = FIP_E_IO;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    if(ret == 0) {
        *err = FIP_E_RESET;
        return 0;
    }
    else {
        if(addr) {
            if(!saw_addr_from_local(saddr, addr)) *err = FIP_E_INVALADDR; /* Set an addr-failure but return buffer. */
        }
        return ret;
    }
}

/* Shuts down specified parts of a connection.
 * Terminates the receiving, sending or both of data on a socket. Further reading/writing will
 * fail. See the IO functions for information on that.
 * It is not required to shut down a socket before closing it but it is recommended.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket which should be shut down.
 * \part describes which parts should be shut down. Can be FIP_RD, FIP_WR or FIP_RDWR.
 *
 * Return value:
 * Does not return anything but sets \err to FIP_E_NONE on success, otherwise
 * an error code is stored.
 *
 * Error codes:
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_NOTCONN: \fd is not connected.
 * FIP_E_FAIL: Invalid error occurred.
 */
void fip_shutdown(fip_err_t *err, fip_socket_t fd, fip_connpart_t part) {
    *err = FIP_E_NONE;
    if(shutdown(fd, FIP_MKPART(part)) == -1) {
        switch(errno) {
            case ENOTSOCK:
            case EBADF:
                *err = FIP_E_BADFD;
                return;
            case ENOTCONN:
                *err = FIP_E_NOTCONN;
                return;
            default:
                *err = FIP_E_FAIL;
                return;
        }
    }
}

/* Closes a connection.
 * Finally closes a socket, thus, it cannot be used anymore. Every socket should be closed
 * to free the internal resources, though, all sockets are closed on program exit automatically.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket.
 *
 * Return value:
 * Does not return anything, but \err is set to FIP_E_NONE on success, otherwise
 * the error code is stored.
 *
 * Error codes:
 * FIP_E_SIGFLOOD: Operation got sigflooded. This does only occur if LINGER is set to >0.
 * FIP_E_IO: IO error occurred.
 * FIP_E_FAIL: Invalid error occurred.
 */
void fip_close(fip_err_t *err, fip_socket_t fd) {
    unsigned int eintr_count = 0;

    *err = FIP_E_NONE;
    eintr_again:
    if(close(fd) == -1) {
        switch(errno) {
            case EINTR: /* The close() call was interrupted by a signal. */
                if(fip_eintr_count == FIP_EINTR_ENDLESS) goto eintr_again;
                    else if(++eintr_count >= fip_eintr_count) {
                        *err = FIP_E_SIGFLOOD;
                        return;
                    }
                    else goto eintr_again;
            case EIO: /* An I/O error occurred. Occurres only on UNIX sockets.
                       * Socket is NOT closed!
                       */
                *err = FIP_E_IO;
                return;
            case EBADF:
                /* badf means file already closed => ignore */
                return;
            default:
                *err = FIP_E_FAIL;
                return;
        }
    }
}

/* System dependent function to set a socket option.
 * * options.h/c provides a system independent wrapper.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket.
 * \level describes the socket level.
 * \opt is the socket option.
 * \val is a pointer to the value which should be set.
 * \size is the size of the data pointed to by \val.
 *
 * Return value:
 * Returns 0 on failure and 1 on success.
 *
 * Error codes:
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_BADARG: Either the option/level is not known or an argument points into invalid memory space.
 *               Additionally the size argument could be invalid.
 * FIP_E_FAIL: Unknown error occurred.
 */
bool fip_setsockopt(fip_err_t *err, fip_socket_t fd, signed int level, signed int opt, const void *val, unsigned int size) {
    if(setsockopt(fd, level, opt, val, size) != 0) {
        switch(errno) {
            case EBADF: /* The argument fd is not a valid descriptor. */
            case ENOTSOCK: /*  The argument fd is a file, not a socket. */
                *err = FIP_E_BADFD;
                return 0;
            case EFAULT: /* The address pointed to by val is not in a valid part of
                            the process address space. For getsockopt(), this error may
                            also be returned if size is not in a valid part of the
                            process address space. */
                *err = FIP_E_BADARG;
                return 0;
            case EINVAL: /* size invalid in setsockopt(). */
            case ENOPROTOOPT: /* The option is unknown at the level indicated. */
                *err = FIP_E_BADARG;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
}

/* System dependent function to get a socket option.
 * * options.h/c provides a system independent wrapper.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket.
 * \level describes the socket level.
 * \opt is the socket option.
 * \val is a pointer where to store the result
 * \size is a pointer where to store the size of the result. Must be initialized with the size of \val.
 *
 * Return value:
 * Returns 0 on failure and 1 on success.
 *
 * If \val is too small to hold the value it is truncated. No error is returned. You have to
 * care for enough space!
 *
 * Error codes:
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_BADARG: Either the option/level is not known or an argument points into invalid memory space or
 *               the size argument is invalid.
 * FIP_E_FAIL: Unknown error occurred.
 */
bool fip_getsockopt(fip_err_t *err, fip_socket_t fd, signed int level, signed int opt, void *val, unsigned int *size) {
    if(getsockopt(fd, level, opt, val, size) != 0) {
        switch(errno) {
            case EBADF: /* The argument fd is not a valid descriptor. */
            case ENOTSOCK: /*  The argument fd is a file, not a socket. */
                *err = FIP_E_BADFD;
                return 0;
            case EFAULT: /* The address pointed to by val is not in a valid part of
                            the process address space. For getsockopt(), this error may
                            also be returned if size is not in a valid part of the
                            process address space. */
                *err = FIP_E_BADARG;
                return 0;
            case EINVAL:
                *err = FIP_E_BADARG;
                return 0;
            case ENOPROTOOPT: /* The option is unknown at the level indicated. */
                *err = FIP_E_BADARG;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
}

/* fcntl() get Wrapper.
 * * options.h/c provides a system independent wrapper.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket.
 * \opts points to a signed integer where to store the current options.
 *
 * Return value:
 * Sets \err and returns 0 on failure and 1 on success.
 *
 * Error codes:
 * FIP_E_DENIED: Operation denied. (possibly by locks held by other processes)
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_FAIL: Invalid error occurred.
 * FIP_E_OPNOTSUPP: The system fcntl() function is not available on your machine.
 */
bool fip_fcntl_get(fip_err_t *err, fip_socket_t fd, signed int *opts) {
#ifdef ONS_CONF_HAVE_FCNTL_H
    if((*opts = fcntl(fd, F_GETFL, 0)) == -1) {
        switch(errno) {
            case EPERM:
            case EACCES:
    #if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            case EWOULDBLOCK:
    #endif
            case EAGAIN: /* Operation is prohibited by locks held by other processes. */
                *err = FIP_E_DENIED;
                return 0;
            case ENOTSOCK:
            case EBADF: /* FD is not an open file descriptor. */
                *err = FIP_E_BADFD;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
#else /* ONS_CONF_HAVE_FCNTL_H */
    *err = FIP_E_OPNOTSUPP;
    return 0;
#endif /* ONS_CONF_HAVE_FCNTL_H */
}

/* fcntl() set Wrapper.
 * * options.h/c provides a system independent wrapper.
 *
 * Parameters:
 * \err is a pointer to an error variable where a possible error code is stored.
 * \fd is a socket.
 * \opts specifies the options to set. The current ones are replaced by these.
 *
 * Return value:
 * Sets \err and returns 0 on failure and 1 on success.
 *
 * Error codes:
 * FIP_E_DENIED: Operation denied. (possibly by locks held by other processes)
 * FIP_E_BADFD: \fd is not a valid socket.
 * FIP_E_FAIL: Invalid error occurred.
 * FIP_E_OPNOTSUPP: The system fcntl() function is not available on your machine.
 * FIP_E_BADARG: Invalid options.
 */
bool fip_fcntl_set(fip_err_t *err, fip_socket_t fd, signed int opts) {
#ifdef ONS_CONF_HAVE_FCNTL_H
    if(fcntl(fd, F_SETFL, opts) != 0) {
        switch(errno) {
            case EPERM:
            case EACCES:
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            case EWOULDBLOCK:
#endif
            case EAGAIN: /* Operation is prohibited by locks held by other processes. */
                *err = FIP_E_DENIED;
                return 0;
            case ENOTSOCK:
            case EBADF: /* FD is not an open file descriptor. */
                *err = FIP_E_BADFD;
                return 0;
            case EINVAL: /* Bad options. */
                *err = FIP_E_BADARG;
                return 0;
            default:
                *err = FIP_E_FAIL;
                return 0;
        }
    }
    return 1;
#else /* ONS_CONF_HAVE_FCNTL_H */
    *err = FIP_E_OPNOTSUPP;
    return 0;
#endif /* ONS_CONF_HAVE_FCNTL_H */
}
