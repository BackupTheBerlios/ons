/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 26. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 30. May 2009
 */

/* OS dependant code
 * This file wraps all functions which depend on the platform's API
 * so the other files don't have to bother with that.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "asynchio/asynchio.h"
#include "backend.h"
#include "memoria/memoria.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef ONS_SOCKET_WIN_HEADERS
    #include <windows.h>
    #include <winsock2.h>
#endif
#ifdef ONS_SOCKET_BERKELEY_HEADERS
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <netinet/in.h>
#endif
#ifdef ONS_SOCKET_IOCTL
    #include <sys/ioctl.h>
#endif
#ifdef ONS_SOCKET_FCNTL
    #include <fcntl.h>
#endif

#ifndef O_NONBLOCK
    #ifdef O_NDELAY
        #define O_NONBLOCK O_NDELAY
    #endif
#endif

#if defined(IPV6_BINDV6ONLY) && !defined(IPV6_V6ONLY)
    #define IPV6_V6ONLY IPV6_BINDV6ONLY
#endif

#ifndef AF_UNIX
    #if defined(AF_FILE)
        #define AF_UNIX AF_FILE
    #elif defined(AF_LOCAL)
        #define AF_UNIX AF_LOCAL
    #endif
#endif

#ifndef PF_UNIX
    #if defined(PF_FILE)
        #define PF_UNIX PF_FILE
    #elif defined(PF_LOCAL)
        #define PF_UNIX PF_LOCAL
    #endif
#endif

#ifndef SHUT_RD
    #ifdef SD_RECEIVE
        #define SHUT_RD SD_RECEIVE
    #endif
#endif

#ifndef SHUT_WR
    #ifdef SD_SEND
        #define SHUT_WR SD_SEND
    #endif
#endif

#ifndef SHUT_RDWR
    #ifdef SD_BOTH
        #define SHUT_RDWR SD_BOTH
    #endif
#endif


/* This defines to a "case EAGAIN" line and if EWOULDBLOCK
 * is also defined with another value it is also catched with this line.
 * Use it like:
 *  switch(errno) {
 *      case ASYN_OS_EAGAIN:
 *          --Do your stuff.--
 *  }
 */
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
    #define ASYN_OS_EAGAIN EWOULDBLOCK: case EAGAIN
#else
    #define ASYN_OS_EAGAIN EAGAIN
#endif



/* This immediately recalls a syscall when it failed with EINTR.
 * Handling of EINTR is weird, however, we decided to recall a syscall
 * until it is not interrupted. We know NO reason why this behaviour
 * should not be used.
 *
 * \syscall has to be the full syscall which is expected to fail when
 * it returns 0.
 */
#ifdef ONS_SOCKET_WIN_HEADERS
    #define ASYN_OS_SYSCALL(syscall) do { if( syscall ) break; } while(WSAGetLastError() == WSAEINTR)
#else
    #define ASYN_OS_SYSCALL(syscall) do { if( syscall ) break; } while(errno == EINTR)
#endif


#if 0
/* Converts a raw address into the classic BSD socket structures and vice versa.
 * The result of the first function has later to be freed.
 */
#define asyn_os_bsa_freesaddr(addr) mem_free(addr)
static struct sockaddr *asyn_os_bsa_mksaddr(unsigned int atype, const asyn_addr_t *addr) {
    struct sockaddr *saddr;

    if(atype == ASYN_OS_UXADDR) {
        saddr = mem_zmalloc(sizeof(struct sockaddr_un));
#ifdef ONS_SOCKET_ALEN
        ((struct sockaddr_un*)saddr)->sun_len = sizeof(struct sockaddr_un);
#endif
        ((struct sockaddr_un*)saddr)->sun_family = AF_UNIX;
        memcpy(((struct sockaddr_un*)saddr)->sun_path,
                addr->addr.ux.path,
                MEM_MIN(sizeof(addr->addr.ux.path), sizeof(((struct sockaddr_un*)saddr)->sun_path)));
        ((struct sockaddr_un*)saddr)->sun_path[sizeof(((struct sockaddr_un*)saddr)->sun_path) - 1] = 0;
    }
    else if(atype == ASYN_OS_V4ADDR) {
        saddr = mem_zmalloc(sizeof(struct sockaddr_in));
#ifdef ONS_SOCKET_ALEN
        ((struct sockaddr_in*)saddr)->sin_len = sizeof(struct sockaddr_in);
#endif
        ((struct sockaddr_in*)saddr)->sin_family = AF_INET;
        ((struct sockaddr_in*)saddr)->sin_port = addr->addr.v4.port;
        ((struct sockaddr_in*)saddr)->sin_addr.s_addr = addr->addr.v4.ip;
    }
    else if(atype == ASYN_OS_V6ADDR) {
        saddr = mem_zmalloc(sizeof(struct sockaddr_in6));
#ifdef ONS_SOCKET_ALEN
        ((struct sockaddr_in6*)saddr)->sin6_len = sizeof(struct sockaddr_in6);
#endif
        /* flowinfo and scope_id are not set by us. */
        ((struct sockaddr_in6*)saddr)->sin6_family = AF_INET6;
        ((struct sockaddr_in6*)saddr)->sin6_port = addr->addr.v6.port;
        memcpy(((struct sockaddr_in6*)saddr)->sin6_addr.s6_addr,
               addr->addr.v6.ip,
               MEM_MIN(sizeof(((struct sockaddr_in6*)saddr)->sin6_addr.s6_addr), sizeof(addr->addr.v6.ip)));
    }
    else return NULL;
    return saddr;
}
static void asyn_os_bsa_readsaddr(struct sockaddr *saddr, size_t size, unsigned int *atype, asyn_addr_t *addr) {
    if(size == sizeof(struct sockaddr_un)) {
        *atype = ASYN_OS_UXADDR;
        memcpy(addr->addr.ux.path,
               ((struct sockaddr_un*)saddr)->sun_path,
               MEM_MIN(sizeof(addr->addr.ux.path), sizeof(((struct sockaddr_un*)saddr)->sun_path)));
        addr->addr.ux.path[sizeof(addr->addr.ux.path) - 1] = 0;
    }
    else if(size == sizeof(struct sockaddr_in)) {
        *atype = ASYN_OS_V4ADDR;
        addr->addr.v4.port = ((struct sockaddr_in*)saddr)->sin_port;
        addr->addr.v4.ip = ((struct sockaddr_in*)saddr)->sin_addr.s_addr;
    }
    else if(size == sizeof(struct sockaddr_in6)) {
        *atype = ASYN_OS_V6ADDR;
        addr->addr.v6.port = ((struct sockaddr_in6*)saddr)->sin6_port;
        memcpy(addr->addr.v6.ip,
               ((struct sockaddr_in6*)saddr)->sin6_addr.s6_addr,
               MEM_MIN(sizeof(((struct sockaddr_in6*)saddr)->sin6_addr.s6_addr), sizeof(addr->addr.v6.ip)));
    }
    else {
        *atype = -1;
        memset(addr, 0, sizeof(asyn_addr_t));
    }
}
static size_t asyn_os_bsa_asize(unsigned int atype) {
    if(atype == ASYN_OS_V4ADDR) return sizeof(struct sockaddr_in);
    else if(atype == ASYN_OS_V6ADDR) return sizeof(struct sockaddr_in6);
    else if(atype == ASYN_OS_UXADDR) return sizeof(struct sockaddr_un);
    else return 0;
}
#endif


unsigned int asyn_os_socket(signed int *fd, unsigned int type, unsigned int domain, unsigned int noinherit) {
    signed int dom, trans, proto, set;

    SUNDRY_ASSERT(fd != NULL);

    /* Set the right \trans and \proto options according to the request type. */
    if(type == ASYN_OS_TCP) {
        trans = SOCK_STREAM;
        proto = IPPROTO_TCP;
    }
    else {
        type = ASYN_OS_UDP;
        trans = SOCK_DGRAM;
        proto = IPPROTO_UDP;
    }

    /* Set the right domain according to the requested domain. */
    if(domain == ASYN_IPV4) dom = PF_INET;
    else {
        domain = ASYN_IPV6;
        dom = PF_INET6;
    }

    /* Set CLOEXEC if the socket() syscalls supports to directly set options. */
#ifdef ONS_SOCKET_EXTSOCK
    if(noinherit) trans |= SOCK_CLOEXEC;
#endif

    /* Execute syscall. */
#ifdef ONS_SOCKET_WIN_HEADERS
    ASYN_OS_SYSCALL(((*fd = socket(dom, trans, proto)) != INVALID_SOCKET));
    if(fd == INVALID_SOCKET) {
        switch(WSAGetLastError()) {
            case WSANOTINITIALISED:
                return ASYN_NOTINIT;
            case WSANETDOWN:
                return ASYN_SYSCALL:
            case WSAEAFNOSUPPORT:
            case WSAESOCKTNOSUPPORT:
            case WSAEPROTOTYPE:
            case WSAEPROTONOSUPPORT:
                return ASYN_NOTSUPP;
            case WSAEMFILE:
                return ASYN_TOOMANY;
            case WSAENOBUFS:
                return ASYN_MEMFAIL;
            default:
                SUNDRY_DEBUG("socket(): Invalid ecode: %d", WSAGetLastError());
                return ASYN_SYSCALL;
        }
    }
#else
    ASYN_OS_SYSCALL(((*fd = socket(dom, trans, proto)) >= 0));
    if(fd < 0) {
        switch(errno) {
            case EAFNOSUPPORT:
            case EPFNOSUPPORT:
            case EINVAL:
            case ESOCKTNOSUPPORT:
            case EPROTONOSUPPORT:
                return ASYN_NOTSUPP;
            case EPERM:
            case EACCES:
                return ASYN_DENIED;
            case EMFILE:
            case ENFILE:
                return ASYN_TOOMANY;
            case ENOBUFS:
            case ENOMEM:
                return ASYN_MEMFAIL;
            default:
                SUNDRY_DEBUG("socket(): Invalid ecode: %d", errno);
                return ASYN_SYSCALL;
        }
    }
#endif

    /* Next step is to set CLOEXEC on platforms which do not support EXTSOCK.
     * We depend on the fcntl() or ioctl() syscall here.
     * Systems which do not support one of both do not support inherited sockets
     * and therefore this action is unneeded.
     * On windows O_NOINHERIT is available, however, only for files and sockets
     * will never be inherited.
     */
#ifdef ONS_SOCKET_EXTSOCK
    /* Do nothing; it was already set before. */
#elif defined(ONS_SOCKET_FCNTL)
    set = fcntl(*fd, F_GETFL, 0);
    if(set == -1) {
        SUNDRY_DEBUG("fcntl(F_GETFL): Failed directly after socket(): %d", errno);
        close(*fd);
        return ASYN_SYSCALL;
    }
    set |= FD_CLOEXEC;
    if(fcntl(*fd, F_SETFL, set) != 0) {
        SUNDRY_DEBUG("fcntl(F_SETFL | FD_CLOEXEC): Failed directly after socket(): %d", errno);
        close(*fd);
        return ASYN_SYSCALL;
    }
#elif defined(ONS_SOCKET_IOCTL)
    if(noinherit && ioctl(*fd, FIOCLEX, NULL) == -1) {
        switch(errno) {
            case EBADF:
            case EINVAL:
                close(*fd);
                return ASYN_SYSCALL;
            default:
                SUNDRY_DEBUG("ioctl(FIOCLEX): Invalid ecode: %d", errno);
                close(*fd);
                return ASYN_SYSCALL;
        }
    }
#endif

    /* Now disable the DualStack mode. */
#ifdef ONS_SOCKET_WIN_HEADERS
    if(setsockopt(*fd, IPPROTO_IPV6, IPV6_V6ONLY, &set, sizeof(set)) == SOCKET_ERROR) {
        SUNDRY_DEBUG("setsockopt(IPV6_V6ONLY): Invalid ecode: %d", WSAGetLastError());
#else
    if(setsockopt(*fd, IPPROTO_IPV6, IPV6_V6ONLY, &set, sizeof(set)) != 0) {
        SUNDRY_DEBUG("setsockopt(IPV6_V6ONLY): Invalid ecode: %d", errno);
#endif
        /* There is no suitable error. This should never happen. */
        close(*fd);
        return ASYN_SYSCALL;
    }

    /* Everything fine. Return. */
    return ASYN_DONE;
}


unsigned int asyn_os_setnblock(signed int fd, unsigned int set) {
    unsigned int ret;

#if defined(ONS_SOCKET_FCNTL)
    signed int val = fcntl(fd, F_GETFL, 0);
    if(val == -1) {
        switch(errno) {
            case EPERM:
            case EACCES:
            case ASYN_OS_EAGAIN:
                /* Operation is prohibited by locks held by other processes. */
                return ASYN_E_DENIED;
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_CASE(EBADF) /* FD is not an open file descriptor. */
                return ASYN_E_BADFD;
            ASYN_OS_CASE(EINVAL) /* Bad options. */
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("fcntl(F_SETFL): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    set |= FD_CLOEXEC;
    if(fcntl(*fd, F_SETFL, set) != 0) {
        SUNDRY_DEBUG("fcntl(F_SETFL | FD_CLOEXEC): Failed directly after socket(): %d", errno);
        close(*fd);
        return ASYN_SYSCALL;
    }
#elif defined(ONS_SOCKET_IOCTL)
    signed int val;
    val = !!set;
    if(ioctl(fd, FIONBIO, &val) == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF)
            ASYN_OS_CASE(EINVAL)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("ioctl(FIONBIO): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
#elif defined(ONS_SOCKET_IOCTLSOCKET)
    unsigned long val;
    val = !!set;
    if(ioctlsocket(fd, FIONBIO, &val) != 0) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            default:
                SUNDRY_DEBUG("ioctl(FIONBIO): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
#endif
}











































#if 0
unsigned int asyn_os_bsa_socket(signed int *fd, unsigned int sdomain, unsigned int stype) {
    signed int dom, trans, proto;
    unsigned int ret, set;

    SUNDRY_ASSERT(fd != NULL);

    if(sdomain == ASYN_OS_BSA_IPV4) {
        dom = PF_INET;
        set = -1;
    }
    else if(sdomain == ASYN_OS_BSA_IPV6) {
        dom = PF_INET6;
        set = 1;
    }
    else if(sdomain == ASYN_OS_BSA_IPV46) {
        dom = PF_INET6;
        set = 0;
    }
    else {
        SUNDRY_DEBUG("asyn_os_bsa_socket() was called with invalid socket domain.");
        return ASYN_E_NOTSUPP;
    }

    if(stype == ASYN_OS_BSA_UDP) {
        trans = SOCK_DGRAM;
        proto = IPPROTO_UDP;
    }
    else if(stype == ASYN_OS_BSA_TCP) {
        trans = SOCK_STREAM;
        proto = IPPROTO_TCP;
    }
    else {
        SUNDRY_DEBUG("asyn_os_bsa_socket() was called with invalid socket type.");
        return ASYN_E_NOTSUPP;
    }

    ASYN_OS_SYSCALL(((*fd = socket(dom, trans, proto)) != -1));
    if(*fd == -1) {
        switch(ASYN_OS_GETERR()) {
            /* Syscall was interrupted. */
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSANETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(EAFNOSUPPORT)      /* The implementation does not support the specified address family. */
            ASYN_OS_CASE(EPFNOSUPPORT)      /* We handle protocol and address families the same way here. */
            ASYN_OS_CASE(EINVAL)            /* Unknown protocol, or protocol family not available. */
            ASYN_OS_CASE(ESOCKTNOSUPPORT)   /* Socket type (transmission method) not supported */
            ASYN_OS_CASE(EPROTONOSUPPORT)   /* The protocol type or the specified protocol is not supported within this domain. */
            ASYN_OS_WCASE(WSAEAFNOSUPPORT)
            ASYN_OS_WCASE(WSAESOCKTNOSUPPORT)
            ASYN_OS_WCASE(WSAEPROTOTYPE)
            ASYN_OS_WCASE(WSAEPROTONOSUPPORT)
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES)           /* Permission to create a socket of the specified type and/or protocol is denied. */
                return ASYN_E_DENIED;
            ASYN_OS_CASE(EMFILE)            /* Process' file table overflow. */
            ASYN_OS_WCASE(WSAEMFILE)
                return ASYN_E_FDPFULL;
            ASYN_OS_CASE(ENFILE)            /* The system's limit on the total number of open files has been reached. */
                return ASYN_E_FDSFULL;
            ASYN_OS_CASE(ENOBUFS)
            ASYN_OS_CASE(ENOMEM)            /* Insufficient memory is available. */
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            default:
                SUNDRY_DEBUG("socket(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }

    /* This can return: ASYN_E_*: *_BADFD, *_BADARG, *_SYSCALL
     * All of them are fatal errors here and we treat them as
     * ASYN_E_SYSCALL.
     */
    if(set != -1) {
        ret = asyn_os_bsa_setsockopt(*fd, ASYN_OS_BSA_V6ONLY, &set, sizeof(set));
        if(ret != ASYN_E_SUCCESS) {
            asyn_os_bsa_close(*fd);
            return ASYN_E_SYSCALL;
        }
    }

    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_socketpair(signed int *fd1, signed int *fd2) {
    signed int tfd[2], res;

    SUNDRY_ASSERT(fd1 != NULL);
    SUNDRY_ASSERT(fd2 != NULL);

#ifdef ONS_SOCKET_WIN_HEADERS
    /* On Windows there is no "socketpair" call. We just connect two sockets together. */
    signed int listener;
    unsigned int ret, atype;
    asyn_os_addr_t addr;
    memset(&addr, 0, sizeof(addr));
    if((ret = asyn_os_bsa_socket(&listener, ASYN_OS_BSA_IPV6, ASYN_OS_BSA_TCP)) != ASYN_E_SUCCESS) return ret;
    if((ret = asyn_os_bsa_socket(&tfd[0], ASYN_OS_BSA_IPV6, ASYN_OS_BSA_TCP)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        return ret;
    }
    if((ret = asyn_os_bsa_bind(listener, ASYN_OS_V6ADDR, &addr)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    if((ret = asyn_os_bsa_listen(listener, 1)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    if((ret = asyn_os_bsa_getsockname(listener, &atype, &addr)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    if((ret = asyn_os_bsa_setnblock(tfd[0], 1)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    if((ret = asyn_os_bsa_connect(tfd[0], atype, &addr)) != ASYN_E_SUCCESS && ret != ASYN_E_NONBLOCK) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    if((ret = asyn_os_bsa_accept(tfd[0], &tfd[1], NULL, NULL)) != ASYN_E_SUCCESS) {
        asyn_os_bsa_close(listener);
        asyn_os_bsa_close(tfd[0]);
        return ret;
    }
    /* Remove the "connection succeeded" message from the buffer. */
    asyn_os_bsa_recv(tfd[0], &ret, sizeof(ret), NULL, NULL);
    asyn_os_bsa_setnblock(tfd[0], 0);
    asyn_os_bsa_close(listener);
#else
    ASYN_OS_SYSCALL(((res = socketpair(PF_UNIX, SOCK_STREAM, 0, tfd)) != -1));
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
                return ASYN_E_INTR;
            ASYN_OS_CASE(EAFNOSUPPORT)      /* The implementation does not support the specified address family. */
            ASYN_OS_CASE(EPFNOSUPPORT)      /* PF/AF are handled the same way here. */
            ASYN_OS_CASE(EPROTONOSUPPORT)   /* The protocol type or the specified protocol is not supported within this domain. */
            ASYN_OS_CASE(EOPNOTSUPP)        /* The specified protocol does not support creation of socket pairs. */
            ASYN_OS_CASE(ESOCKTNOSUPPORT)   /* Socket type (transmission method) not supported */
            ASYN_OS_CASE(EINVAL)            /* Unknown protocol, or protocol family not available. */
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(EMFILE)            /* Process file table overflow. */
                return ASYN_E_FDPFULL;
            ASYN_OS_CASE(ENFILE)            /* The system limit on the total number of open files has been reached. */
                return ASYN_E_FDSFULL;
            ASYN_OS_CASE(ENOBUFS)
            ASYN_OS_CASE(ENOMEM)            /* Insufficient memory is available. */
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES)           /* Permission to create a socket of the specified type and/or protocol is denied. */
                return ASYN_E_DENIED;
            default:
                SUNDRY_DEBUG("socketpair(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
#endif

    *fd1 = tfd[0];
    *fd2 = tfd[1];
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_bind(signed int fd, unsigned int atype, const asyn_addr_t *addr) {
    struct sockaddr *saddr;
    size_t size;
    signed int res;

    size = asyn_os_bsa_asize(atype);
    if(!size) return ASYN_E_NOTSUPP;
    saddr = asyn_os_bsa_mksaddr(atype, addr);
    if(!saddr) return ASYN_E_NOTSUPP;

    ASYN_OS_SYSCALL(((res = bind(fd, saddr, size)) != -1));
    asyn_os_bsa_freesaddr(saddr);
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(EBADF) /* \fd is not a valid descriptor. */
            ASYN_OS_CASE(ENOTSOCK) /* \fd is a descriptor for a file, not a socket. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES) /* The address is protected, and the user is not the superuser. */
            ASYN_OS_WCASE(WSAEACCES)
                return ASYN_E_DENIED;
            ASYN_OS_CASE(EADDRINUSE) /* The given address is already in use. */
            ASYN_OS_WCASE(WSAEADDRINUSE)
                return ASYN_E_ADDRINUSE;
            ASYN_OS_CASE(EINVAL) /* The socket is already bound to an address or invalid argument. */
            ASYN_OS_WCASE(WSAEINVAL)
                return ASYN_E_ALREADY;
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EADDRNOTAVAIL)
            ASYN_OS_WCASE(WSAEADDRNOTAVAIL)
                return ASYN_E_ADDRNOTAVAIL;
            ASYN_OS_WCASE(WSAEFAULT)
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(EIO) /* File: I/O error */
                return ASYN_E_IO;
            ASYN_OS_CASE(ENOTDIR) /* File: Not a directory */
                return ASYN_E_NOTDIR;
            ASYN_OS_CASE(EISDIR) /* File: Is a directory */
                return ASYN_E_ISDIR;
            ASYN_OS_CASE(ENOSPC) /* File: No space left on device */
                return ASYN_E_NOSPACELEFT;
            ASYN_OS_CASE(EROFS) /* File: Read-only file system */
                return ASYN_E_ROFS;
            ASYN_OS_CASE(EMLINK) /* File: Too many links */
            ASYN_OS_CASE(ELOOP) /* File: Too many symbolic links encountered */
                return ASYN_E_LOOP;
            ASYN_OS_CASE(ENAMETOOLONG) /* File: File name too long */
                return ASYN_E_FNAMETOOLONG;
            ASYN_OS_CASE(EDQUOT) /* File: Quota exceeded */
                return ASYN_E_QUOTA;
            default:
                SUNDRY_DEBUG("bind(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_listen(signed int fd, unsigned int backlog) {
    signed int res;

    /* truncate backlog (unix defines a signed integer here!) */
    if(backlog > INT_MAX) backlog = INT_MAX;

    ASYN_OS_SYSCALL(((res = listen(fd, (signed int)backlog)) != -1));
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSAEMFILE)
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EADDRINUSE) /* Another socket is already listening on the same port. */
            ASYN_OS_WCASE(WSAEADDRINUSE)
                return ASYN_E_ADDRINUSE;
            ASYN_OS_CASE(EBADF) /* The argument sockfd is not a valid descriptor. */
            ASYN_OS_CASE(ENOTSOCK) /* The argument sockfd is not a socket. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(EINVAL)
                return ASYN_E_ALREADY;
            ASYN_OS_CASE(EISCONN)
            ASYN_OS_WCASE(WSAEISCONN)
                return ASYN_E_ISCONN;
            ASYN_OS_CASE(EFAULT)
            ASYN_OS_CASE(EOPNOTSUPP) /* The socket is not of a type that supports the listen() operation. */
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAEOPNOTSUPP)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("listen(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_connect(signed int fd, unsigned int atype, const asyn_addr_t *addr) {
    signed int res;
    struct sockaddr *saddr;
    size_t size;

    size = asyn_os_bsa_asize(atype);
    if(!size) return ASYN_E_NOTSUPP;
    saddr = asyn_os_bsa_mksaddr(atype, addr);
    if(!saddr) return ASYN_E_NOTSUPP;

    ASYN_OS_SYSCALL(((res = connect(fd, saddr, size)) != -1));
    asyn_os_bsa_freesaddr(saddr);
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES) /* The user tried to connect to a broadcast address without having
                            the socket broadcast flag enabled or the connection request
                            failed because of a local firewall rule.
                            On Unix Sockets it can indicate a read-only FS or directory. */
            ASYN_OS_WCASE(WSAEACCES)
                return ASYN_E_DENIED;
            ASYN_OS_CASE(ENOTSOCK) /* The file descriptor is not associated with a socket. */
            ASYN_OS_CASE(EBADF) /* The file descriptor is not a valid index in the descriptor table. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(EHOSTDOWN) /* Host is down. */
                return ASYN_E_HOSTDOWN;
            ASYN_OS_CASE(EHOSTUNREACH) /* No route to host. */
            ASYN_OS_WCASE(WSAEHOSTUNREACH)
                return ASYN_E_HOSTUNREACH;
            ASYN_OS_CASE(EAFNOSUPPORT) /* The passed address didn't have the correct address family in its sa_family field. */
            ASYN_OS_CASE(EINVAL)
            ASYN_OS_CASE(EFAULT) /* Invalid addess. */
            ASYN_OS_WCASE(WSAEAFNOSUPPORT)
            ASYN_OS_WCASE(WSAEFAULT)
            ASYN_OS_WCASE(WSAEINVAL)
                return ASYN_E_NOTSUPP;
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EAGAIN) /* no more free local ports */
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(ENETDOWN) /* Network is down. */
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(ENETUNREACH) /* Network unreachable. */
            ASYN_OS_WCASE(WSAENETUNREACH)
                return ASYN_E_NETUNREACH;
            ASYN_OS_CASE(EISCONN) /* Already connected. */
            ASYN_OS_WCASE(WSAEISCONN)
                return ASYN_E_ISCONN;
            ASYN_OS_CASE(ETIMEDOUT) /* Operation timed out. */
            ASYN_OS_WCASE(WSAETIMEDOUT)
                return ASYN_E_TIMEDOUT;
            ASYN_OS_CASE(ECONNREFUSED) /* Other side refused connection. */
            ASYN_OS_WCASE(WSAECONNREFUSED)
                return ASYN_E_REFUSED;
            ASYN_OS_CASE(EALREADY) /* Other connection is already in progress. */
            ASYN_OS_WCASE(WSAEALREADY)
                return ASYN_E_ALREADY;
            ASYN_OS_CASE(EINPROGRESS) /* Operation continued in background. */
            ASYN_OS_WCASE(WSAEINPROGRESS)
            ASYN_OS_WCASE(WSAEWOULDBLOCK)
                return ASYN_E_NONBLOCK;
            ASYN_OS_CASE(EADDRINUSE) /* Address already in use. */
            ASYN_OS_WCASE(WSAEADDRINUSE)
                return ASYN_E_ADDRINUSE;
            ASYN_OS_WCASE(WSAEADDRNOTAVAIL)
                return ASYN_E_ADDRNOTAVAIL;
            ASYN_OS_CASE(ENOENT) /* File: No such file or directory */
            ASYN_OS_CASE(ENAMETOOLONG) /* File: File name too long */
                return ASYN_E_NOFILE;
            ASYN_OS_CASE(ENOTDIR) /* File: Not a directory */
                return ASYN_E_NOTDIR;
            ASYN_OS_CASE(EISDIR) /* File: Is a directory */
                return ASYN_E_ISDIR;
            ASYN_OS_CASE(EMLINK) /* File: Too many links */
            ASYN_OS_CASE(ELOOP) /* File: Too many symbolic links encountered */
                return ASYN_E_LOOP;
            ASYN_OS_CASE(EROFS) /* File: read only filesystem */
                return ASYN_E_ROFS;
            default:
                SUNDRY_DEBUG("connect(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_accept(signed int fd, signed int *new_fd, unsigned int *atype, asyn_addr_t *addr) {
    signed int res;
    socklen_t size = ASYN_OS_SOCKBUF;
    char caddr[ASYN_OS_SOCKBUF];
    struct sockaddr *saddr = (struct sockaddr*)caddr;

    ASYN_OS_SYSCALL(((res = accept(fd, saddr, &size)) != -1));
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            ASYN_OS_CASE(EWOULDBLOCK)
#endif
            ASYN_OS_CASE(EAGAIN) /* The socket is marked non-blocking and no connections are present
                            to be accepted. */
            ASYN_OS_WCASE(WSAEWOULDBLOCK)
                return ASYN_E_NONBLOCK;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EOPNOTSUPP) /* The referenced socket is not of type SOCK_STREAM. */
            ASYN_OS_CASE(EFAULT) /* The addr argument is not in a writable part of the user address space. */
            ASYN_OS_CASE(EINVAL) /* Socket is not listening for connections, or addrlen is invalid
                            (e.g., is negative). */
            ASYN_OS_WCASE(WSAEFAULT)
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAEOPNOTSUPP)
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(EMFILE) /* The per-process limit of open file descriptors has been reached. */
            ASYN_OS_WCASE(WSAEMFILE)
                return ASYN_E_FDPFULL;
            ASYN_OS_CASE(ENFILE) /* The  system  limit  on  the  total number of open files has been reached. */
                return ASYN_E_FDSFULL;
            ASYN_OS_CASE(ENOTSOCK) /* The descriptor references a file, not a socket. */
            ASYN_OS_CASE(EBADF) /* The descriptor is invalid. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(ENOBUFS)
            ASYN_OS_CASE(ENOMEM) /* Not enough free memory. This often means that the memory allocation
                            is limited by the socket buffer limits, not by the system memory. */
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EPROTO) /* Protocol error. */
                return ASYN_E_IO;
            ASYN_OS_CASE(ECONNABORTED) /* A connection has been aborted. */
                return ASYN_E_ABORTED;
            ASYN_OS_WCASE(WSAECONNRESET)
                return ASYN_E_RESET;
            ASYN_OS_CASE(EPERM) /* Firewall rules forbid connection. */
                return ASYN_E_DENIED;
            default:
                /* In addition, network errors for the new socket and as defined for the
                 * protocol may be returned.  Various Linux kernels can return other
                 * errors such as ENOSR, ESOCKTNOSUPPORT, EPROTONOSUPPORT, ETIMEDOUT.
                 */
                SUNDRY_DEBUG("accept(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    *new_fd = res;
    if(addr) asyn_os_bsa_readsaddr(saddr, size, atype, addr);
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_send(signed int fd, const void *buf, size_t *size, unsigned int atype, const asyn_addr_t *addr) {
    signed int res;
    struct sockaddr *saddr;
    size_t asize = 0;

    if(addr) {
        asize = asyn_os_bsa_asize(atype);
        if(!asize) return ASYN_E_NOTSUPP;
        saddr = asyn_os_bsa_mksaddr(atype, addr);
        if(!saddr) return ASYN_E_NOTSUPP;
    }
    else saddr = NULL;

    ASYN_OS_SYSCALL(((res = sendto(fd, buf, *size, 0, saddr, asize)) != -1));
    asyn_os_bsa_freesaddr(saddr);
    if(res == -1) {
        *size = 0;
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSAENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_WCASE(WSAEADDRNOTAVAIL)
                return ASYN_E_ADDRNOTAVAIL;
            ASYN_OS_WCASE(WSAETIMEDOUT)
                return ASYN_E_TIMEDOUT;
            ASYN_OS_CASE(EINVAL) /* Invalid argument passed. */
            ASYN_OS_CASE(EFAULT) /* An invalid user space address was specified for a parameter. */
            ASYN_OS_CASE(EAFNOSUPPORT) /* (*^+) Address family not supported by protocol */
            ASYN_OS_CASE(EOPNOTSUPP) /* Some bit in the flags argument is inappropriate for the socket type. */
            ASYN_OS_CASE(EDESTADDRREQ) /* The socket is not connection-mode, and no peer address is set. */
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAEFAULT)
            ASYN_OS_WCASE(WSAEOPNOTSUPP)
            ASYN_OS_WCASE(WSAEAFNOSUPPORT)
            ASYN_OS_WCASE(WSAEDESTADDRREQ)
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(ENOTSOCK) /* The argument \fd is not a socket. */
            ASYN_OS_CASE(EBADF) /* An invalid descriptor was specified. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            ASYN_OS_CASE(EWOULDBLOCK)
#endif
            ASYN_OS_CASE(EAGAIN) /* The socket is marked non-blocking and the requested operation would block. */
            ASYN_OS_WCASE(WSAEWOULDBLOCK)
                return ASYN_E_NONBLOCK;
            ASYN_OS_CASE(ENOBUFS) /* The output queue for a network interface was full. This gener-
                             ally indicates that the interface has stopped sending, but may
                             be caused by transient congestion. (Normally, this does not
                             occur in Linux. Packets are just silently dropped when a device
                             queue overflows.) */
            ASYN_OS_CASE(ENOMEM) /* No memory available. */
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(EMSGSIZE) /* The socket type requires that message be sent atomically, and
                              the size of the message to be sent made this impossible. */
            ASYN_OS_WCASE(WSAEMSGSIZE)
                return ASYN_E_MSGSIZE;
            ASYN_OS_CASE(ECONNREFUSED) /* Other side refused the network connection. */
                return ASYN_E_REFUSED;
            ASYN_OS_CASE(EISCONN) /* The connection-mode socket was connected already but a recipient
                             was specified. (Now either this error is returned, or the
                             recipient specification is ignored.) */
                return ASYN_E_ISCONN;
            ASYN_OS_CASE(ENOTCONN) /* The socket is not connected, and no target has been given. */
            ASYN_OS_WCASE(WSAENOTCONN)
                return ASYN_E_NOTCONN;
            ASYN_OS_CASE(EPIPE) /* The local end has been shut down on a connection oriented
                           socket. In this case the process will also receive a SIGPIPE
                           unless MSG_NOSIGNAL is set. */
                return ASYN_E_PIPE;
            ASYN_OS_CASE(ECONNRESET) /* Connection reset by peer. */
            ASYN_OS_WCASE(WSAECONNRESET)
                return ASYN_E_RESET;
            ASYN_OS_CASE(EHOSTDOWN) /* (*^+) Host is down */
                return ASYN_E_HOSTDOWN;
            ASYN_OS_CASE(EHOSTUNREACH) /* (*^+) No route to host */
            ASYN_OS_WCASE(WSAEHOSTUNREACH)
                return ASYN_E_HOSTUNREACH;
            ASYN_OS_CASE(ENETDOWN) /* (*^+) Network is down */
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(ENETUNREACH) /* (*^+) Network is unreachable */
            ASYN_OS_WCASE(WSAENETUNREACH)
                return ASYN_E_NETUNREACH;
            ASYN_OS_CASE(ENETRESET) /* (*^+#) Network dropped connection because of reset */
            ASYN_OS_WCASE(WSAENETRESET)
                return ASYN_E_NETRESET;
            ASYN_OS_CASE(ECONNABORTED) /* (*^+#) Software caused connection abort */
            ASYN_OS_WCASE(WSAECONNABORTED)
                return ASYN_E_ABORTED;
            ASYN_OS_CASE(ESHUTDOWN) /* (*^+) Cannot send after transport endpoint shutdown */
            ASYN_OS_WCASE(WSAESHUTDOWN)
                return ASYN_E_SHUTDOWN;
            ASYN_OS_CASE(EPERM) /* (*^+) Operation not permitted */
            ASYN_OS_CASE(EACCES) /* (*^+) Permission denied */
            ASYN_OS_WCASE(WSAEACCES)
                return ASYN_E_DENIED;
            ASYN_OS_CASE(EROFS) /* (+) Read-only file system */
                return ASYN_E_ROFS;
            ASYN_OS_CASE(EIO) /* (*^?+) I/O error */
                return ASYN_E_IO;
            ASYN_OS_CASE(ENOENT) /* (*^+) No such file or directory */
            ASYN_OS_CASE(ENAMETOOLONG) /* (+) File name too long */
                return ASYN_E_NOFILE;
            ASYN_OS_CASE(ENOTDIR) /* (+) Not a directory */
                return ASYN_E_NOTDIR;
            ASYN_OS_CASE(EISDIR) /* (+) Is a directory */
                return ASYN_E_ISDIR;
            ASYN_OS_CASE(EMLINK) /* (+) Too many links */
            ASYN_OS_CASE(ELOOP) /* (+) Too many symbolic links encountered */
                return ASYN_E_LOOP;
            ASYN_OS_CASE(EFBIG) /* (+) File too large */
                return ASYN_E_TOOBIG;
            ASYN_OS_CASE(ENOSPC) /* (+) No space left on device */
                return ASYN_E_NOSPACELEFT;
            ASYN_OS_CASE(EDQUOT) /* (+) Quota exceeded */
                return ASYN_E_QUOTA;
            default:
                SUNDRY_DEBUG("sendto(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    *size = res;
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_recv(signed int fd, void *buf, size_t *size, unsigned int *atype, asyn_addr_t *addr) {
    signed int res;
    socklen_t asize = ASYN_OS_SOCKBUF;
    char caddr[ASYN_OS_SOCKBUF];
    struct sockaddr *saddr = (struct sockaddr*)caddr;

    ASYN_OS_SYSCALL(((res = recvfrom(fd, buf, *size, 0, saddr, &asize)) != -1));
    if(res == -1) {
        *size = 0;
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSAETIMEDOUT)
                return ASYN_E_TIMEDOUT;
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            ASYN_OS_CASE(EWOULDBLOCK)
#endif
            ASYN_OS_CASE(EAGAIN) /* The socket is marked non-blocking and the receive operation
                            would block, or a receive timeout has been set and the timeout
                            expired before data was received. */
            ASYN_OS_WCASE(WSAEWOULDBLOCK)
                return ASYN_E_NONBLOCK;
            ASYN_OS_CASE(EINVAL) /* Invalid argument passed. */
            ASYN_OS_CASE(EFAULT) /* An invalid user space address was specified for a parameter. */
            ASYN_OS_CASE(EOPNOTSUPP) /* Some bit in the flags argument is inappropriate for the socket type. */
            ASYN_OS_WCASE(WSAEFAULT)
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAEISCONN)
            ASYN_OS_WCASE(WSAEOPNOTSUPP)
                return ASYN_E_NOTSUPP;
            ASYN_OS_CASE(ENOTSOCK) /* The argument \fd is not a socket. */
            ASYN_OS_CASE(EBADF) /* An invalid descriptor was specified. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(ENOBUFS)
            ASYN_OS_CASE(ENOMEM) /* No memory available. */
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(ENOTCONN) /* The socket is not connected. */
                return ASYN_E_NOTCONN;
            ASYN_OS_CASE(EPIPE) /* The local end has been shut down on a connection oriented
                           socket. In this case the process will also receive a SIGPIPE
                           unless MSG_NOSIGNAL is set. */
                return ASYN_E_PIPE;
            ASYN_OS_CASE(ECONNRESET) /* Connection reset by peer. */
            ASYN_OS_WCASE(WSAECONNRESET)
                return ASYN_E_RESET;
            ASYN_OS_CASE(ENETRESET) /* (*^+#) Network dropped connection because of reset */
            ASYN_OS_WCASE(WSAENETRESET)
                return ASYN_E_NETRESET;
            ASYN_OS_CASE(ECONNABORTED) /* (*^+#) Software caused connection abort */
                return ASYN_E_ABORTED;
            ASYN_OS_CASE(ESHUTDOWN) /* (*^+) Cannot send after transport endpoint shutdown */
            ASYN_OS_WCASE(WSAESHUTDOWN)
                return ASYN_E_SHUTDOWN;
            ASYN_OS_CASE(EPERM) /* (*^+) Operation not permitted */
            ASYN_OS_CASE(EACCES) /* (*^+) Permission denied */
                return ASYN_E_DENIED;
            ASYN_OS_CASE(EIO) /* (*^?+) I/O error */
                return ASYN_E_IO;
            default:
                SUNDRY_DEBUG("recvfrom(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    else if(res == 0) {
        *size = 0;
        return ASYN_E_RESET;
    }

    if(addr) asyn_os_bsa_readsaddr(saddr, asize, atype, addr);
    *size = res;
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_shutdown(signed int fd, unsigned int kread, unsigned int kwrite) {
    signed int part, res;

    part = 0;
    if(kread) {
        if(kwrite) part = SHUT_RDWR;
        else part = SHUT_RD;
    }
    else if(kwrite) part = SHUT_WR;
    /* Nothing to do, success. */
    else return ASYN_E_SUCCESS;

    ASYN_OS_SYSCALL(((res = shutdown(fd, part)) != -1));
    if(res == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR)
            ASYN_OS_WCASE(WSAEINTR)
                return ASYN_E_INTR;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_CASE(EBADF)
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_CASE(ENOTCONN)
            ASYN_OS_WCASE(WSAENOTCONN)
                return ASYN_E_NOTCONN;
            default:
                SUNDRY_DEBUG("shutdown(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


void asyn_os_bsa_close(signed int fd) {
    signed int ret;

    eintr_again:
#ifdef ONS_SOCKET_WIN_HEADERS
    ret = 1;
    setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, ret, sizeof(ret));
    ASYN_OS_SYSCALL(((ret = closesocket(fd)) != -1));
#else
    ASYN_OS_SYSCALL(((ret = close(fd)) != -1));
#endif
    if(ret == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EINTR) /* The close() call was interrupted by a signal. */
                /* There is no reason to every return here, close() has to succeed! */
            ASYN_OS_WCASE(WSAEINTR)
                goto eintr_again;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return;
            ASYN_OS_WCASE(WSAENETDOWN)
                return;
            ASYN_OS_CASE(EIO) /* An I/O error occurred. Assume socket is closed. */
                return;
            ASYN_OS_CASE(EBADF)
            ASYN_OS_WCASE(WSAENOTSOCK)
                /* badf means file already closed => ignore */
                return;
            default:
                SUNDRY_DEBUG("close(): Invalid ecode: %d", ASYN_OS_GETERR());
                /* Assume \fd is closed now. */
                return;
        }
    }
}


unsigned int asyn_os_bsa_setsockopt(signed int fd, unsigned int opt, const void *val, unsigned int size) {
    signed int level, option;

    if(opt == ASYN_OS_BSA_REUSEADDR) {
        level = SOL_SOCKET;
        option = SO_REUSEADDR;
    }
    else if(opt == ASYN_OS_BSA_V6ONLY) {
        level = IPPROTO_IPV6;
        option = IPV6_V6ONLY;
    }
    else {
        return ASYN_E_NOTSUPP;
    }

    if(setsockopt(fd, level, option, val, size) != 0) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF) /* The argument fd is not a valid descriptor. */
            ASYN_OS_CASE(ENOTSOCK) /*  The argument fd is a file, not a socket. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_CASE(EFAULT) /* The address pointed to by val is not in a valid part of
                            the process address space. For getsockopt(), this error may
                            also be returned if size is not in a valid part of the
                            process address space. */
            ASYN_OS_CASE(EINVAL) /* size invalid in setsockopt(). */
            ASYN_OS_CASE(ENOPROTOOPT) /* The option is unknown at the level indicated. */
            ASYN_OS_WCASE(WSAEFAULT)
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAENOPROTOOPT)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("setsockopt(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_getsockopt(signed int fd, unsigned int opt, void *val, unsigned int *size) {
    signed int level, option;

    if(opt == ASYN_OS_BSA_REUSEADDR) {
        level = SOL_SOCKET;
        option = SO_REUSEADDR;
    }
    else if(opt == ASYN_OS_BSA_V6ONLY) {
        level = IPPROTO_IPV6;
        option = IPV6_V6ONLY;
    }
    else {
        return ASYN_E_NOTSUPP;
    }

    if(getsockopt(fd, level, option, val, size) != 0) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF) /* The argument fd is not a valid descriptor. */
            ASYN_OS_CASE(ENOTSOCK) /*  The argument fd is a file, not a socket. */
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_CASE(EFAULT) /* The address pointed to by val is not in a valid part of
                            the process address space. For getsockopt(), this error may
                            also be returned if size is not in a valid part of the
                            process address space. */
            ASYN_OS_CASE(EINVAL) /* size invalid in setsockopt(). */
            ASYN_OS_CASE(ENOPROTOOPT) /* The option is unknown at the level indicated. */
            ASYN_OS_WCASE(WSAEFAULT);
            ASYN_OS_WCASE(WSAEINVAL)
            ASYN_OS_WCASE(WSAENOPROTOOPT)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("setsockopt(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_getpeername(signed int fd, unsigned int *atype, asyn_addr_t *addr) {
    socklen_t asize = ASYN_OS_SOCKBUF;
    char caddr[ASYN_OS_SOCKBUF];
    struct sockaddr *saddr = (struct sockaddr*)caddr;

    if(0 != getpeername(fd, saddr, &asize)) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF)
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_CASE(ENOBUFS)
                return ASYN_E_MEMFAIL;
            ASYN_OS_CASE(ENOTCONN)
            ASYN_OS_WCASE(WSAENOTCONN)
                return ASYN_E_NOTCONN;
            default:
                SUNDRY_DEBUG("getpeername(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }

    asyn_os_bsa_readsaddr(saddr, asize, atype, addr);
    return ASYN_E_SUCCESS;
}


unsigned int asyn_os_bsa_getsockname(signed int fd, unsigned int *atype, asyn_addr_t *addr) {
    socklen_t asize = ASYN_OS_SOCKBUF;
    char caddr[ASYN_OS_SOCKBUF];
    struct sockaddr *saddr = (struct sockaddr*)caddr;

    if(0 != getsockname(fd, saddr, &asize)) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF)
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            ASYN_OS_WCASE(WSAENETDOWN)
                return ASYN_E_NETDOWN;
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_CASE(ENOBUFS)
                return ASYN_E_MEMFAIL;
            default:
                SUNDRY_DEBUG("getpeername(): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }

    asyn_os_bsa_readsaddr(saddr, asize, atype, addr);
    return ASYN_E_SUCCESS;
}


#if defined(ONS_SOCKET_FCNTL)
unsigned int asyn_os_bsa_gfcntl(signed int fd, unsigned int *opts) {
    signed int r_opts;

    if((r_opts = fcntl(fd, F_GETFL, 0)) == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES)
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            ASYN_OS_CASE(EWOULDBLOCK)
#endif
            ASYN_OS_CASE(EAGAIN) /* Operation is prohibited by locks held by other processes. */
                return ASYN_E_DENIED;
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_CASE(EBADF) /* FD is not an open file descriptor. */
                return ASYN_E_BADFD;
            default:
                SUNDRY_DEBUG("fcntl(F_GETFL): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }

    *opts = 0;
    if(r_opts & O_NONBLOCK) *opts |= ASYN_OS_BSA_NBLOCK;
    if(r_opts & FD_CLOEXEC) *opts |= ASYN_OS_BSA_CLOEXEC;

    return ASYN_E_SUCCESS;
}
#endif /* defined(ONS_SOCKET_FCNTL) */


#if defined(ONS_SOCKET_FCNTL)
unsigned int asyn_os_bsa_sfcntl(signed int fd, unsigned int opts) {
    signed int r_opts;

    /* Don't care for errors. Significant errors will reoccur below in the second fcntl() call. */
    r_opts = fcntl(fd, F_GETFL, 0);
    if(opts & ASYN_OS_BSA_SET) {
        if(opts & ASYN_OS_BSA_NBLOCK) r_opts |= O_NONBLOCK;
        if(opts & ASYN_OS_BSA_CLOEXEC) r_opts |= FD_CLOEXEC;
    }
    else if(opts & ASYN_OS_BSA_UNSET) {
        if(opts & ASYN_OS_BSA_NBLOCK) r_opts &= ~O_NONBLOCK;
        if(opts & ASYN_OS_BSA_CLOEXEC) r_opts &= ~FD_CLOEXEC;
    }
    else return ASYN_E_SUCCESS;

    if(fcntl(fd, F_SETFL, r_opts) != 0) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EPERM)
            ASYN_OS_CASE(EACCES)
#if defined(EWOULDBLOCK) && !(EWOULDBLOCK==EAGAIN)
            ASYN_OS_CASE(EWOULDBLOCK)
#endif
            ASYN_OS_CASE(EAGAIN) /* Operation is prohibited by locks held by other processes. */
                return ASYN_E_DENIED;
            ASYN_OS_CASE(ENOTSOCK)
            ASYN_OS_CASE(EBADF) /* FD is not an open file descriptor. */
                return ASYN_E_BADFD;
            ASYN_OS_CASE(EINVAL) /* Bad options. */
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("fcntl(F_SETFL): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
}
#endif /* defined(ONS_SOCKET_FCNTL) */


unsigned int asyn_os_bsa_setnblock(signed int fd, unsigned int set) {
    unsigned int ret;

#if defined(ONS_SOCKET_FCNTL)
    if(set) ret = asyn_os_bsa_sfcntl(fd, ASYN_OS_BSA_SET | ASYN_OS_BSA_NBLOCK);
    else ret = asyn_os_bsa_sfcntl(fd, ASYN_OS_BSA_UNSET | ASYN_OS_BSA_NBLOCK);
    return ret;
#elif defined(ONS_SOCKET_IOCTL)
    signed int val;
    val = !!set;
    if(ioctl(fd, FIONBIO, &val) == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF)
            ASYN_OS_CASE(EINVAL)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("ioctl(FIONBIO): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
#elif defined(ONS_SOCKET_IOCTLSOCKET)
    unsigned long val;
    val = !!set;
    if(ioctlsocket(fd, FIONBIO, &val) != 0) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_WCASE(WSANOTINITIALISED)
                return ASYN_E_NOINIT;
            ASYN_OS_WCASE(WSAENOTSOCK)
                return ASYN_E_BADFD;
            default:
                SUNDRY_DEBUG("ioctl(FIONBIO): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
#else
    #error "None of fcntl(), ioctl() or ioctlsocket() are available."
#endif
}


unsigned int asyn_os_bsa_setinherit(signed int fd, unsigned int set) {
    unsigned int ret;

#if defined(ONS_SOCKET_FCNTL)
    if(set) ret = asyn_os_bsa_sfcntl(fd, ASYN_OS_BSA_SET | ASYN_OS_BSA_CLOEXEC);
    else ret = asyn_os_bsa_sfcntl(fd, ASYN_OS_BSA_UNSET | ASYN_OS_BSA_CLOEXEC);
    return ret;
#elif defined(ONS_SOCKET_IOCTL)
    signed int val;
    val = !!set;
    if(ioctl(fd, FIOCLEX, &val) == -1) {
        switch(ASYN_OS_GETERR()) {
            ASYN_OS_CASE(EBADF)
            ASYN_OS_CASE(EINVAL)
                return ASYN_E_NOTSUPP;
            default:
                SUNDRY_DEBUG("ioctl(FIOCLEX): Invalid ecode: %d", ASYN_OS_GETERR());
                return ASYN_E_SYSCALL;
        }
    }
    return ASYN_E_SUCCESS;
#endif
    /* All other systems do not support this. */
    return ASYN_E_SUCCESS;
}

#endif
