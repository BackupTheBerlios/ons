/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 3. June 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
 */

/* Socket option interface */

#define ONS_CONF_ECODES
#include "fip/fip.h"
#include <saw/saw.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef ONS_CONF_HAVE_SYS_TYPES_H
    #include <sys/types.h>
#endif
#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif
#ifdef ONS_CONF_HAVE_FCNTL_H
    #include <fcntl.h>
#endif
#ifdef ONS_CONF_HAVE_SYS_IOCTL_H
    #include <sys/ioctl.h>
#endif

#if defined(ONS_CONF_NONBLOCK_POSIX)
    #define FIP_NONBLOCK O_NONBLOCK
#elif defined(ONS_CONF_NONBLOCK_BSD)
    #define FIP_NONBLOCK O_NDELAY
#endif

#if defined(IPV6_BINDV6ONLY) && !defined(IPV6_V6ONLY)
    #define IPV6_V6ONLY IPV6_BINDV6ONLY
#endif

static bool _fip_optset_bool(ons_err_t *err, fip_socket_t fd, unsigned int opt, va_list *argp) {
    unsigned int val = va_arg(*argp, unsigned int);
    signed int tmp;
    val &= 0x1;
    switch(opt) {
        case FIP_OPT_NONBLOCK:
#if defined(FIP_NONBLOCK)
            if(val) {
                if(!fip_fcntl_get(err, fd, &tmp)) return 0;
                if(!fip_fcntl_set(err, fd, tmp | FIP_NONBLOCK)) return 0;
            }
            else {
                if(!fip_fcntl_get(err, fd, &tmp)) return 0;
                if(!fip_fcntl_set(err, fd, tmp & ~FIP_NONBLOCK)) return 0;
            }
#else
            /* make signed */
            tmp = val;
            if(ioctl(fd, FIONBIO, &tmp) == -1) return 0;
#endif
            return 1;
        case FIP_OPT_BROADCAST:
            if(!fip_setsockopt(err, fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(unsigned int))) return 0;
            return 1;
        case FIP_OPT_KEEPALIVE:
            if(!fip_setsockopt(err, fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(unsigned int))) return 0;
            return 1;
        case FIP_OPT_REUSEADDR:
            if(!fip_setsockopt(err, fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(unsigned int))) return 0;
            return 1;
        case FIP_OPT_DUALSTACK:
            val = !val;
            if(!fip_setsockopt(err, fd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(unsigned int))) return 0;
            return 1;
        default:
            *err = ONS_E_UNKNOWN;
            return 0;
    }
}

static bool _fip_optget_bool(ons_err_t *err, fip_socket_t fd, unsigned int opt, va_list *argp) {
    unsigned int *val = va_arg(*argp, unsigned int*);
    unsigned int tsize = sizeof(unsigned int);
    signed int tmp;
    switch(opt) {
        case FIP_OPT_NONBLOCK:
#if defined(FIP_NONBLOCK)
            if(!fip_fcntl_get(err, fd, &tmp)) return 0;
            if((tmp & FIP_NONBLOCK) == FIP_NONBLOCK) *val = 1;
            else *val = 0;
            return 1;
#else
            *err = ONS_E_OPNOTSUPP;
            return 0;
#endif
        case FIP_OPT_BROADCAST:
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_BROADCAST, &val, &tsize)) return 0;
            if(*val != 0) *val = 1;
            return 1;
        case FIP_OPT_KEEPALIVE:
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_KEEPALIVE, &val, &tsize)) return 0;
            if(*val != 0) *val = 1;
            return 1;
        case FIP_OPT_REUSEADDR:
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_REUSEADDR, &val, &tsize)) return 0;
            if(*val != 0) *val = 1;
            return 1;
        case FIP_OPT_ACCEPT:
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_ACCEPTCONN, &val, &tsize)) return 0;
            if(*val != 0) *val = 1;
            return 1;
        case FIP_OPT_DUALSTACK:
            if(!fip_getsockopt(err, fd, IPPROTO_IPV6, IPV6_V6ONLY, &val, &tsize)) return 0;
            if(*val != 0) *val = 1;
            *val = !*val;
            return 1;
        default:
            *err = ONS_E_BADARG;
            return 0;
    }
    *err = ONS_E_BADARG;
    return 0;
}

static bool _fip_optget_misc(ons_err_t *err, fip_socket_t fd, unsigned int opt, va_list *argp) {
    unsigned int *uval;
    signed int *sval;
    unsigned int tsize;
    saw_addr_t *addr;
    char sawbuf[SAW_ADDR_LOCSIZE];

    switch(opt) {
        case FIP_OPT_ERROR:
            uval = va_arg(*argp, unsigned int*);
            tsize = sizeof(signed int);
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_ERROR, &sval, &tsize)) return 0;
            errno = *sval;
            *uval = fip_mkerrno(*sval);
            return 1;
        case FIP_OPT_TYPE:
            uval = va_arg(*argp, unsigned int*);
            tsize = sizeof(signed int);
            if(!fip_getsockopt(err, fd, SOL_SOCKET, SO_TYPE, &sval, &tsize)) return 0;
            *uval = FIP_MKTRANS_REV(*sval);
            if(*uval == 255) { *err = ONS_E_UNKNOWN; return 0; }
            return 1;
        case FIP_OPT_PEERNAME:
            tsize = SAW_ADDR_LOCSIZE;
            if(0 != getpeername(fd, (saw_addr_local_t*)sawbuf, &tsize)) {
                switch(errno) {
                    case EBADF:
                    case ENOTSOCK:
                        *err = ONS_E_BADFD;
                        return 0;
                    case ENOBUFS:
                        *err = ONS_E_MEMFAIL;
                        return 0;
                    case ENOTCONN:
                        *err = ONS_E_NOTCONN;
                        return 0;
                    default:
                        ONS_iDEBUG("_fip_optget_misc(): Invalid ecode", errno);
                        *err = ONS_E_FAIL;
                        return 0;
                }
            }
            addr = va_arg(*argp, saw_addr_t*);
            if(!saw_addr_from_local((saw_addr_local_t*)sawbuf, addr)) { *err = ONS_E_UNKNOWN; return 0; }
            return 1;
        case FIP_OPT_SOCKNAME:
            tsize = SAW_ADDR_LOCSIZE;
            if(0 != getsockname(fd, (saw_addr_local_t*)sawbuf, &tsize)) {
                switch(errno) {
                    case EBADF:
                    case ENOTSOCK:
                        *err = ONS_E_BADFD;
                        return 0;
                    case ENOBUFS:
                        *err = ONS_E_MEMFAIL;
                        return 0;
                    default:
                        ONS_iDEBUG("_fip_optget_misc(): Invalid ecode", errno);
                        *err = ONS_E_FAIL;
                        return 0;
                }
            }
            addr = va_arg(*argp, saw_addr_t*);
            if(!saw_addr_from_local((saw_addr_local_t*)sawbuf, addr)) { *err = ONS_E_UNKNOWN; return 0; }
            return 1;
        default:
            *err = ONS_E_BADARG;
            return 0;
    }
}

bool fip_optset(ons_err_t *err, fip_socket_t fd, unsigned int opt, ...) {
    va_list argp;
    unsigned int ret = 0;

    va_start(argp, opt);
    switch(opt) {
        case FIP_OPT_NONBLOCK:
        case FIP_OPT_BROADCAST:
        case FIP_OPT_KEEPALIVE:
        case FIP_OPT_REUSEADDR:
        case FIP_OPT_DUALSTACK:
            ret = _fip_optset_bool(err, fd, opt, &argp);
            break;
        default:
            *err = ONS_E_BADARG;
            return 0;
    }
    va_end(argp);
    return ret;
}

bool fip_optget(ons_err_t *err, fip_socket_t fd, unsigned int opt, ...) {
    va_list argp;
    unsigned int ret = 0;

    va_start(argp, opt);
    switch(opt) {
        case FIP_OPT_NONBLOCK:
        case FIP_OPT_BROADCAST:
        case FIP_OPT_KEEPALIVE:
        case FIP_OPT_REUSEADDR:
        case FIP_OPT_ACCEPT:
        case FIP_OPT_DUALSTACK:
            ret = _fip_optget_bool(err, fd, opt, &argp);
            break;
        case FIP_OPT_ERROR:     /* Takes internal signed, external unsigned. */
        case FIP_OPT_TYPE:      /* same */
        case FIP_OPT_PEERNAME:
        case FIP_OPT_SOCKNAME:
            ret = _fip_optget_misc(err, fd, opt, &argp);
        default:
            *err = ONS_E_BADARG;
            return 0;
    }
    va_end(argp);
    return ret;
}

