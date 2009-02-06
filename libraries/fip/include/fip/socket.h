/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 3. June 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 2. January 2009
 */

/* Socket functions.
 * This header provides a socket interface similar to the Berkeley Sockets.
 */

#include <fip/misc.h>

#ifndef ONS_INCLUDED_fip_socket_h
#define ONS_INCLUDED_fip_socket_h
ONS_EXTERN_C_BEGIN


#include <saw/saw.h>
#include <fip/error.h>

#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif
#ifdef ONS_CONF_HAVE_NETINET_IN_H
    #include <netinet/in.h>
#endif

/* Socket datatype. */
typedef signed int fip_socket_t;
typedef fip_socket_t fip_fd_t;

/* Invalid socket (fip_fd) */
#define FIP_INVALID_SOCKET (-1)

/* Transmission types (unsigned int) */
#define FIP_TRANS_STREAM 0
#define FIP_TRANS_DGRAM 1

/* Protocol types (unsigned int) */
#define FIP_PROTO_STD 0
#define FIP_PROTO_TCP 1
#define FIP_PROTO_UDP 2

/* Parts of a full-duplex connection. */
typedef enum fip_connpart_t {
    /* The FIP_XX_SYS values are only used in the converters. You shouldn't use them. */
    /* SHUT_RD/WR/RDWR shall be defined in sys/socket.h (POSIX). */
#ifdef SHUT_RD
    #define FIP_RD_SYS SHUT_RD
#else
    #define FIP_RD_SYS 0
#endif
#ifdef SHUT_WR
    #define FIP_WR_SYS SHUT_WR
#else
    #define FIP_WR_SYS 1
#endif
#ifdef SHUT_RDWR
    #define FIP_RDWR_SYS SHUT_RDWR
#else
    #define FIP_RDWR_SYS 2
#endif
    FIP_RD = 0,         /* Read part */
    FIP_WR = 1,         /* Write part */
    FIP_RDWR = 2        /* Superset */
} fip_connpart_t;

/* These converters convert the FIP constants into system internal constants.
 * The system internal datatypes are not known, therefore we use these converters
 * to avoid unnecessary "unsigned int", "signed int", "unsigned long", "signed long",
 * "size_t", "ssize_t", "socksize_t" ... incompatibilities.
 *
 * The FIP functions do NOT accept system internal values. Please use these converters
 * only if you require the system values to use them with other libraries/functions than FIP.
 *
 * The *_REV functions are "reversed" functions which do the same but in the other direction.
 */

/* Converts a protocol family (domain) into the system internal value.
 * Takes as argument a Libsaw domain (eg. SAW_ADDR4_DOM).
 *
 * (Libsaw provides direct access to internal values => nothing to do. (signed in))
 */
#define FIP_MKDOM(dom) (dom)
#define FIP_MKDOM_REV(dom) (dom)

/* Converts a transmission method into the system internal one.
 * Takes as argument an unsigned int transmission method (eg. FIP_TRANS_STREAM).
 * The return value on an unknown transmission method is 255.
 * Please use ONLY the FIP_TRANS_* constants here.
 *
 * SOCK_STREAM/DGRAM/... shall be defined in sys/socket.h (POSIX).
 */
#define _FIP_TRANS_INVALID (255) /* Returned if "trans" is not known. */
#define FIP_MKTRANS(trans) (((trans) == FIP_TRANS_STREAM)? \
                                SOCK_STREAM: \
                                ( \
                                    ((trans) == FIP_TRANS_DGRAM)? \
                                    SOCK_DGRAM: \
                                    _FIP_TRANS_INVALID \
                                ) \
                           )
#define FIP_MKTRANS_REV(trans) (((trans) == SOCK_STREAM)? \
                                FIP_TRANS_STREAM: \
                                ( \
                                    ((trans) == SOCK_DGRAM)? \
                                    FIP_TRANS_DGRAM: \
                                    _FIP_TRANS_INVALID \
                                ) \
                               )

/* Converts a FIP protocol into a system internal one. It returns FIP_PROTO_STD if the
 * protocol is not known, thus, please use only FIP_PROTO_* constants here.
 *
 * IPPROTO_TCP/UDP shall be defined in sys/socket.h (POSIX).
 */
#define _FIP_PROTO_INVALID (FIP_PROTO_STD) /* Returned if "proto" is not known. */
#define FIP_MKPROTO(proto) (((proto) == FIP_PROTO_TCP)? \
                                IPPROTO_TCP: \
                                ( \
                                    ((proto) == FIP_PROTO_UDP)? \
                                    IPPROTO_UDP: \
                                    FIP_PROTO_STD \
                                ) \
                           )
#define FIP_MKPROTO_REV(proto) (((proto) == IPPROTO_TCP)? \
                                FIP_PROTO_TCP: \
                                ( \
                                    ((proto) == IPPROTO_UDP)? \
                                    FIP_PROTO_UDP: \
                                    FIP_PROTO_STD \
                                ) \
                               )

/* Converts a full-duplex part into the system internal constant.
 * Returns the superset if the part is invalid.
 */
#define FIP_MKPART(part) (((part) == FIP_RD)? \
                                FIP_RD_SYS: \
                                ( \
                                    ((part) == FIP_WR)? \
                                    FIP_WR_SYS: \
                                    FIP_RDWR_SYS \
                                ) \
                         )
#define FIP_MKPART_REV(part) (((part) == FIP_RD_SYS)? \
                                FIP_RD: \
                                ( \
                                    ((part) == FIP_WR_SYS)? \
                                    FIP_WR: \
                                    FIP_RDWR \
                                ) \
                             )

/* Creates a new socket. */
extern fip_socket_t fip_socket(fip_err_t *err, signed int dom, unsigned int trans, unsigned int proto);

/* Creates a new pair of sockets connected to each other. */
extern bool fip_socketpair(fip_err_t *err, unsigned int trans, unsigned int proto, fip_socket_t *fd1, fip_socket_t *fd2);

/* Binds a socket to a particular address. */
extern bool fip_bind(fip_err_t *err, fip_socket_t fd, const saw_addr_t *addr);

/* Creates a listener on a socket. */
extern bool fip_listen(fip_err_t *err, fip_socket_t fd, unsigned int backlog);

/* Connects to a remote socket. */
extern bool fip_connect(fip_err_t *err, fip_socket_t fd, const saw_addr_t *addr);

/* Accepts a connection on a listener. */
extern fip_socket_t fip_accept(fip_err_t *err, fip_socket_t fd, saw_addr_t *addr);

/* Standard IO on a socket. */
extern unsigned int fip_send(fip_err_t *err, fip_socket_t fd, const char *buf, unsigned int bufsize, const saw_addr_t *addr, unsigned int opts);
extern unsigned int fip_recv(fip_err_t *err, fip_socket_t fd, char *buf, unsigned int size, saw_addr_t *addr, unsigned int opts);

/* Shuts down specified parts of a connection. */
extern void fip_shutdown(fip_err_t *err, fip_socket_t fd, fip_connpart_t part);

/* Closes a connection. */
extern void fip_close(fip_err_t *err, fip_socket_t fd);

/* System dependent function to set/get a socket option.
 * Returns 0 on failure and 1 on success.
 * * options.h/c provides a system independent wrapper.
 */
extern bool fip_setsockopt(fip_err_t *err, fip_socket_t fd, signed int level, signed int opt, const void *val, unsigned int size);
extern bool fip_getsockopt(fip_err_t *err, fip_socket_t fd, signed int level, signed int opt, void *val, unsigned int *size);

/* fcntl() get/set wrapper.
 * Sets *err and returns 0 on failure and 1 on success.
 * * options.h/c provides a system independent wrapper.
 */
extern bool fip_fcntl_get(fip_err_t *err, fip_socket_t fd, signed int *opts);
extern bool fip_fcntl_set(fip_err_t *err, fip_socket_t fd, signed int opts);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_socket_h */

