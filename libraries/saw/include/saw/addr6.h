/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 19. April 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* IPv6 helper functions which manage your IPv6 addresses in a
 * simple unique manner.  Structure converters are also part of
 * this file: string to address (pton)
 *            address to string (ntop)
 *            address to local addr-structure (to_local)
 *            local addr-structure to address (from_local)
 *
 * The namespaces are saw_addr6_* and SAW_ADDR6_*.
 * "addr6" stands for "Internet Protocol version 6 Address" and
 * is referred here as "IPv6 address" or similar.
 */

#include <saw/misc.h>

/* Prevent the inclusion of this file if IPv6 is disabled. */
#ifdef ONS_CONF_NO_IPV6
    #define ONS_INCLUDED_saw_addr6_h
#endif

#ifndef ONS_INCLUDED_saw_addr6_h
#define ONS_INCLUDED_saw_addr6_h
ONS_EXTERN_C_BEGIN


#ifdef ONS_CONF_HAVE_NETINET_IN_H
    #include <netinet/in.h> /* Definition of the local datatype. */
#endif

#include <string.h>         /* memcpy() in converter macro. */

#define SAW_ADDR6_FAM ((signed int)AF_INET6)
#define SAW_ADDR6_DOM ((signed int)PF_INET6)
#define SAW_ADDR6_STRLEN ((unsigned int)46)

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(push)
    #pragma pack(1)
#endif
union saw_addr6_t {
    uint8_t addr[16];
    uint16_t word[8];
    uint32_t dword[4];
    uint64_t qword[2];
} ONS_ATTR_PACK;
#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(pop)
#endif

typedef union saw_addr6_t saw_addr6_t;
#define SAW_ADDR6_SIZE ((unsigned int)sizeof(saw_addr6_t))

#define SAW_ADDR6_ANY {.addr = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}
const saw_addr6_t saw_addr6_any;

#define SAW_ADDR6_LOOPBACK {.addr = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}}
#define SAW_ADDR6_LO SAW_ADDR6_LOOPBACK
const saw_addr6_t saw_addr6_loopback;
#define saw_addr6_lo saw_addr6_loopback

#define SAW_ADDR6_INTERFACE_LOCAL {.addr = {255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1}}
#define SAW_ADDR6_IFLO SAW_ADDR6_INTERFACE_LOCAL
const saw_addr6_t saw_addr6_interface_local;
#define saw_addr6_iflo saw_addr6_interface_local

#define SAW_ADDR6_LINK_LOCAL {.addr = {255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1}}
#define SAW_ADDR6_LILO SAW_ADDR6_LINK_LOCAL
const saw_addr6_t saw_addr6_link_local;
#define saw_addr6_lilo saw_addr6_link_local;

/* helper macros */
#define _SAW_ADDR6_RPL(x) {.addr = {255,x,0,0,0,0,0,0,0,0,0,0,0,0,0,2}}

#define SAW_ADDR6_INTERFACE_LOCAL_ROUTERS _SAW_ADDR6_RPL(1)
#define SAW_ADDR6_IFLO_ROUTERS SAW_ADDR6_INTERFACE_LOCAL_ROUTERS
const saw_addr6_t saw_addr6_interface_local_routers;
#define saw_addr6_iflo_routers saw_addr6_interface_local_routers

#define SAW_ADDR6_LINK_LOCAL_ROUTERS _SAW_ADDR6_RPL(2)
#define SAW_ADDR6_LILO_ROUTERS SAW_ADDR6_LINK_LOCAL_ROUTERS
const saw_addr6_t saw_addr6_link_local_routers;
#define saw_addr6_lilo_routers saw_addr6_link_local_routers

#define SAW_ADDR6_SITE_LOCAL_ROUTERS _SAW_ADDR6_RPL(5)
#define SAW_ADDR6_SILO_ROUTERS SAW_ADDR6_SITE_LOCAL_ROUTERS
const saw_addr6_t saw_addr6_site_local_routers;
#define saw_addr6_silo_routers saw_addr6_site_local_routers

typedef struct in6_addr saw_addr6_local_t;
#define SAW_ADDR6_LOCSIZE ((unsigned int)sizeof(saw_addr6_local_t))

/* multicast flags / use them on arg->addr[1] / eg. if(arg->addr[1] & SAW_ADDR6_MC_R) */
#define SAW_ADDR6_MC_T 0x10 /* T-flag */
#define SAW_ADDR6_TRANSIENT SAW_ADDR6_MC_T /* T-flag indicates a transient multicast address. */
#define SAW_ADDR6_MC_P 0x20 /* P-flag */
#define SAW_ADDR6_MC_R 0x40 /* R-flag */

/* multicast scope values / use it on (arg->addr[1] & SAW_ADDR6_SCOPE) */
/* scope mask */
#define SAW_ADDR6_SCOPE 0x0F
/* 0  reserved */
#define SAW_ADDR6_SCOPE_INTERFACE_LOCAL 1 /* 1  Interface-Local scope */
/* Interface-Local scope spans only a single interface on a node
   and is useful only for loopback transmission of multicast. */
#define SAW_ADDR6_SCOPE_LINK_LOCAL 2 /* 2  Link-Local scope */
/* Link-Local multicast scope spans the same topological region as
   the corresponding unicast scope. */
/* 3  reserved */
#define SAW_ADDR6_SCOPE_ADMIN_LOCAL 4 /* 4  Admin-Local scope */
/* Admin-Local scope is the smallest scope that must be
   administratively configured, i.e., not automatically derived
   from physical connectivity or other, non-multicast-related
   configuration. */
#define SAW_ADDR6_SCOPE_SITE_LOCAL 5 /* 5  Site-Local scope */
/* Site-Local scope is intended to span a single site. */
/* 6  (unassigned) */
/* 7  (unassigned) */
#define SAW_ADDR6_SCOPE_ORGANISATION_LOCAL 8 /* 8  Organization-Local scope */
/* Organization-Local scope is intended to span multiple sites
   belonging to a single organization. */
/* 9  (unassigned) */
/* A  (unassigned) */
/* B  (unassigned) */
/* C  (unassigned) */
/* D  (unassigned) */
#define SAW_ADDR6_SCOPE_GLOBAL 0xE /* E  Global scope */
/* F  reserved */

/* String to address and vice versa converters. See source file for string syntaxes. */
extern bool saw_addr6_pton(const char *ipstring, saw_addr6_t *dest);
extern void saw_addr6_ntop(const saw_addr6_t *ip, char *ipstring);

/* extern void saw_addr6_copy(const saw_addr6_t *first, saw_addr6_t *second); */
#define saw_addr6_copy(first, second) _saw_addr6_copy(((const saw_addr6_t*)(first)), ((saw_addr6_t*)(second)))
#define _saw_addr6_copy(first, second) (memcpy(second, first, SAW_ADDR6_SIZE))
#define saw_addr6_cpy(first, second) saw_addr6_copy((first), (second))

/* extern bool saw_addr6_compare(const saw_addr6_t *first, const saw_addr6_t *second); */
#define saw_addr6_compare(first, second) _saw_addr6_compare(((const saw_addr6_t*)(first)), ((saw_addr6_t*)(second)))
#define _saw_addr6_compare(first, second) (0 == memcmp(first, second, SAW_ADDR6_SIZE))
#define saw_addr6_cmp(first, second) saw_addr6_compare((first), (second))

/* extern bool saw_addr6_isany(const saw_addr6_t *arg); */
#define saw_addr6_isany(arg) _saw_addr6_isany(((saw_addr6_t*)(arg)))
#define _saw_addr6_isany(arg) (arg->qword[0] == 0 && arg->qword[1] == 0)

/* extern bool saw_addr6_isloopback(const saw_addr6_t *arg); */
#define saw_addr6_isloopback(arg) _saw_addr6_isloopback(((saw_addr6_t*)(arg)))
#define _saw_addr6_isloopback(arg) (saw_addr6_compare(arg, &saw_addr6_loopback))
#define saw_addr6_islo(arg) saw_addr6_isloopback(arg)

/* extern bool saw_addr6_islinklocal(const saw_addr6_t *arg); */
#define saw_addr6_islinklocal(arg) _saw_addr6_islinklocal(((saw_addr6_t*)(arg)))
#define _saw_addr6_islinklocal(arg) (arg->addr[0] == 0xFE && (arg->addr[1] & 0xC0) == 128)
#define saw_addr6_isll(arg) saw_addr6_islinklocal(arg)

/* extern bool saw_addr6_ismulticast(const saw_addr6_t *arg); */
#define saw_addr6_ismulticast(arg) _saw_addr6_ismulticast(((saw_addr6_t*)(arg)))
#define _saw_addr6_ismulticast(arg) (arg->addr[0] == 0xFF)
#define saw_addr6_ismc(arg) saw_addr6_ismulticast(arg)

/* extern bool saw_addr6_issolicitedmulticast(const saw_addr6_t *arg); */
#define saw_addr6_issolicitedmulticast(arg) _saw_addr6_issolicitedmulticast(((saw_addr6_t*)(arg)))
#define _saw_addr6_issolicitedmulticast(arg) ( \
                                                arg->addr[0] == 0xFF && \
                                                arg->addr[1] == 0x02 && \
                                                arg->word[1] == 0x00 && \
                                                arg->word[2] == 0x00 && \
                                                arg->word[3] == 0x00 && \
                                                arg->word[4] == 0x00 && \
                                                arg->addr[10] == 0x00 && \
                                                arg->addr[11] == 0x01 && \
                                                arg->addr[12] == 0xFF \
                                             )
#define saw_addr6_issmc(arg) saw_addr6_issolicitedmulticast(arg)

/* extern bool saw_addr6_is4mapped(const saw_addr6_t *arg); */
#define saw_addr6_is4mapped(arg) _saw_addr6_is4mapped(((saw_addr6_t*)(arg)))
#define _saw_addr6_is4mapped(arg) (arg->qword[0] == 0 && arg->word[4] == 0 && arg->word[5] == 0xFFFF)
#define saw_addr6_is4m(arg) saw_addr6_is4mapped(arg)

/* extern void saw_addr6_to_local(const saw_addr6_t *ip, saw_addr6_local_t *local); */
#define saw_addr6_to_local(ip, local) _saw_addr6_to_local(((const saw_addr6_t*)(ip)), ((saw_addr6_local_t*)(local)))
#define _saw_addr6_to_local(ip, local) (memset(local, 0, SAW_ADDR6_LOCSIZE), memcpy(&local->s6_addr, ip->addr, SAW_ADDR6_SIZE))

/* extern void saw_addr6_from_local(const saw_addr6_local_t *local, saw_addr6_t *pbuf); */
#define saw_addr6_from_local(local, pbuf) _saw_addr6_from_local(((const saw_addr6_local_t*)(local)), ((saw_addr6_t*)(pbuf)))
#define _saw_addr6_from_local(local, pbuf) (memcpy(pbuf->addr, &(local->s6_addr), SAW_ADDR6_SIZE))


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_addr6_h */

