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

/* Socket option interface (rev29 old options) */

#include <fip/misc.h>

#ifndef ONS_INCLUDED_fip_options_h
#define ONS_INCLUDED_fip_options_h
ONS_EXTERN_C_BEGIN


/* Socket options.
 * This file tries to wrap the common socket options on all platforms. However, there are
 * plenty of additional socket options which are not available on all platforms. These
 * options have a TODO state and will be available if a appropriate way is found to port
 * these options to all supported platforms.
 *
 * Linux options:
 *   IPPROTO_IP:
 *     IP_MTU: (integer) returns the current MTU.
 *     IP_MTUDISCOVER: Enables MTU discovery. The following options are allowed:
 *                      - IP_PMTUDISC_WANT   Use per-route settings.
 *                      - IP_PMTUDISC_DONT   Never do Path MTU Discovery.
 *                      - IP_PMTUDISC_DO     Always do Path MTU Discovery.
 *                      - IP_PMTUDISC_PROBE  Set DF but ignore Path MTU.
 *                     The Don't Fragment (DF) bit is set if MTU-Disc is enabled.
 *     IP_OPTIONS: (char[40]) Sets the IP options field.
 *     IP_PKTINFO: (bool) enables or disables the pktinfo ancillary data.
 *     IP_RECVERR: (bool) enables receive of extended error messages as ancillary data.
 *     IP_RECVOPTS: (bool) enables receive of ip options as ancillary data.
 *     IP_RETOPTS: (bool) enables receive of raw ip options as ancillary data.
 *     IP_RECVTOS: (bool) enables receive of the TOS as ancillary data.
 *     IP_RECVTTL: (bool) enables receive of the TTL as ancillary data.
 *     IP_TOS: (flag/unknown) Sets the Type Of Service to one of these:
 *                     - IPTOS_LOWDELAY to minimize delays for interactive traffic
 *                     - IPTOS_THROUGHPUT to optimize throughput
 *                     - IPTOS_RELIABILITY to optimize for reliability
 *                     - IPTOS_MINCOST where slow transmission doesn't matter
 *     IP_TTL: (unknown) set or receive the Time To Live.
 *   IPPROTO_IPV6:
 *     IPV6_ADDRFORM: (integer) Converts the socket into another address family.
 *     IPV6_MTU: Same as IP_MTU but for ipv6.
 *     IPV6_MTU_DISCOVER: likewise
 *     IPV6_PKTINFO: likewise
 *     IPV6_RECVERR: likewise
 *     IPV6_RTHDR, IPV6_AUTHHDR, IPV6_DSTOPS, IPV6_HOPOPTS, IPV6_FLOWINFO, IPV6_HOPLIMIT:
 *       - Enable receiving of protocol information as ancillary data.
 *     IPV6_UNICAST_HOPS: (integer) Sets the limit for unicast hops.
 *     IPV6_V6ONLY or IPV6_BINDV6ONLY: (bool) Disable dual stack.
 *   SOL_SOCKET:
 *     SO_ACCEPTCONN: (bool) Returns whether the socket is a listener.
 *     SO_BINDTODEVICE: (string, eg., "eth0") Bind socket to device.
 *     SO_BROADCAST: (bool) Set or unset the broadcast flag.
 *     SO_DEBUG: (bool) Enable socket debugging.
 *     SO_ERROR: (integer) Returns and clears last socket error.
 *     SO_DONTROUTE: (bool) Enable don't-route option.
 *     SO_KEEPALIVE: (bool) Enable sending of keepalive packets.
 *     SO_LINGER: (struct linger) Set shutdown/close timeouts.
 *     SO_OOBINLINE: (bool) Receive OutOfBand data in main stream.
 *     SO_PASSCRED: (bool) Receive credentials over ancillary data.
 *     SO_PEERCRED: (struct ucred) Return the credentials of the foreign socket. Only valid on
 *                                 AF_UNIX socketpair() sockets.
 *     SO_PRIORITY: (integer) Set the priority of the packet (QOS/TOS).
 *     SO_RCVBUF: (integer) Set internal receive buffer (doubled value).
 *     SO_RCVBUFFORCE: (integer) Allows to force bigger receive buffers.
 *     SO_RCVLOWAT: (integer) Until which buffer size should the kernel retain the data.
 *     SO_RCVTIMEO: (struct timeval) Set the timeout of a recv() call.
 *     SO_SNDBUF: (integer) Set internal send buffer.
 *     SO_SNDBUFFORCE: (integer) Allows to force bigger send buffers.
 *     SO_SNDLOWAT: (integer) Until which buffer size should the kernel retain the data.
 *     SO_SNDTIMEO: (struct timeval) Set the timeout of a send() call.
 *     SO_REUSEADDR: (bool) Enables less strict rules for socket binding.
 *     SO_TIMESTAMP: (bool) Enables receive of timestamps in ancillary data.
 *     SO_TYPE: (integer) Returns the socket type.
 *   IPPROTO_UDP:
 *     UDP_CORK: (bool) Collects data and sends it as single packet when the option is disabled.
 *   IPPROTO_TCP:
 *     TCP_CORK: same as UDP_CORK for TCP.
 *     TCP_DEFER_ACCEPT: unknown behaviour.
 *     TCP_INFO: Returns a "struct tcp_info" about the socket.
 *     TCP_KEEPCNT: (integer) numer of keepalives sent before connection is dropped.
 *     TCP_KEEPIDLE: (integer) Time after which keepalive packets should be sent.
 *     TCP_KEEPINTVL: (integer) Timeinterval between two keepalive packets.
 *     TCP_LINGER2: The lifetime of orphaned FIN_WAIT2 state sockets
 *     TCP_MAXSEG: (integer) maximum segment value.
 *     TCP_NODELAY: (integer) Disable Naggle-Algorithm, sent segments immediately.
 *     TCP_QUICKACK: Toggle quick TCP-ACK acknowledgement.
 *     TCP_SYNCNT: (integer<=255) Set maximum transmission of SYN until connection abort.
 *     TCP_WINDOW_CLAMP: unknwon behaviour.
 */


#include <fip/error.h>
#include <fip/socket.h>

/* All socket options are listed below. They are set/unset through the fip_optset interface. The parameter
 * which you should pass to the function is described behind every option.
 * The following abbreviations are used (An "*" behind the abbr. means that it is the respective pointer type):
 *      - (BOOL): Parameter is an unsigned int. 1 means true and 0 means false.
 *      - (UINT): Parameter is an unsigned int.
 *      - (SINT): Parameter is an signed int.
 *      - (U64T): Parameter is an uint64_t.
 *      - (CHAR): A char type.
 *      - ([pug]ID): They stand for the pid_t/uid_t/gid_t datatypes. They are platform dependent and only used in the testing options.
 *      - (ADDR): A pointer to a libsaw address.
 *      - (O_GET): Only valid in fip_optget().
 * The fip_optget function needs a pointer to the specific datatype if it is not already a pointer. The fip_setopt functions does not need
 * a pointer unless the datatype is explicitly defined as pointer.
 * optset parameters can be constant, optget parameters CAN'T.
 */

/* Platform independent (ported to all supported platforms) socket options. */
#define FIP_OPT_NONBLOCK             1 /* Sets the socket into nonblocking state. (BOOL) */
#define FIP_OPT_BROADCAST            2 /* Enables to send broadcasts on this socket. Most systems support this only on UDP sockets. (BOOL) */
#define FIP_OPT_KEEPALIVE            3 /* Enables sending of keepalives. (BOOL) */
#define FIP_OPT_REUSEADDR            4 /* Enable the Reuseaddr option. (BOOL) */
#define FIP_OPT_ACCEPT               5 /* Checks whether the socket is a listening one. (BOOL) (O_GET) */
#define FIP_OPT_ERROR                6 /* Saves the last socket error and clears the pending error. (UINT) (O_GET) */
#define FIP_OPT_TYPE                 7 /* Returns the socket type. (UINT) (O_GET) */
#define FIP_OPT_PEERNAME             8 /* Returns the address of the peer. (ADDR*) (O_GET) */
#define FIP_OPT_SOCKNAME             9 /* Returns the address of the local socket. (ADDR*) (O_GET) */
#define FIP_OPT_DUALSTACK           10 /* Enables the IPv4/IPv6 Dualstack mode. (BOOL) */

extern bool fip_optset(ons_err_t *err, fip_socket_t fd, unsigned int opt, ...);
extern bool fip_optget(ons_err_t *err, fip_socket_t fd, unsigned int opt, ...);

/* The following options are transmission options (TOPT) which can be passed to fip_recv/send. */
#define FIP_TOPT_NONE           0x0000 /* No option. */
#define FIP_TOPT_OOB            0x0002 /* Receive/Send OOB data. */

/* Clears all invalid/unknown options. */
/* Required headers for system dependent codes. */
#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif
#define FIP_MKTOPTS(opts) ( \
                                (((opts) & FIP_TOPT_OOB)?MSG_OOB:0x0) | \
                                0x0 \
                          )


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_options_h */

