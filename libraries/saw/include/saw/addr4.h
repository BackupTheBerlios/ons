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

/* IPv4 helper functions which manage your IPv4 addresses in a
 * simple plain and clear manner. Structure converters are also
 * part of this file:
 *          string to address (pton)
 *          address to string (ntop)
 *          address to local addr-structure (to_local)
 *          local addr-structure to address (from_local)
 *
 * The namespaces are saw_addr4_* and SAW_ADDR4_*.
 * "addr4" stands for "Internet Protocol version 4 Address" and
 * is referred here as "IPv4 address" or similar.
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_addr4_h
#define ONS_INCLUDED_saw_addr4_h
ONS_EXTERN_C_BEGIN


#ifdef ONS_CONF_HAVE_NETINET_IN_H
    #include <netinet/in.h>     /* definition of local address structure */
#endif

#include <string.h>             /* memset() in converter macro. */

#define SAW_ADDR4_FAM ((signed int)AF_INET)
#define SAW_ADDR4_DOM ((signed int)PF_INET)
#define SAW_ADDR4_STRLEN ((unsigned int)16)

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(push)
    #pragma pack(1)
#endif
union saw_addr4_t {
    uint32_t addr;
    uint16_t word[2];
    uint8_t byte[4];
} ONS_ATTR_PACK;
#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(pop)
#endif

typedef union saw_addr4_t saw_addr4_t;
#define SAW_ADDR4_SIZE ((unsigned int)sizeof(saw_addr4_t))

#define SAW_ADDR4_ANY {.addr = 0x00000000}
extern const saw_addr4_t saw_addr4_any;

#define SAW_ADDR4_LOOPBACK {.addr = 0x7f000001}
#define SAW_ADDR4_LO SAW_ADDR4_LOOPBACK
extern const saw_addr4_t saw_addr4_loopback;
#define saw_addr4_lo saw_addr4_loopback

#define SAW_ADDR4_BROADCAST {.addr = 0xffffffff}
#define SAW_ADDR4_BC SAW_ADDR4_BROADCAST
extern const saw_addr4_t saw_addr4_broadcast;
#define saw_addr4_bc saw_addr4_broadcast

typedef struct in_addr saw_addr4_local_t;
#define SAW_ADDR4_LOCSIZE ((unsigned int)sizeof(saw_addr4_local_t))

/* String to address and vice versa converters. See source file for string syntaxes. */
extern bool saw_addr4_pton(const char *ipstring, saw_addr4_t *dest);
extern void saw_addr4_ntop(const saw_addr4_t *ip, char *pbuf);

/* Casts a saw_addr4_t into a uint8_t const pointer which is used in the macros below. */
#define _saw_addr4_u8p_cast(data) (((const saw_addr4_t*)(data))->byte)

/* extern bool saw_addr4_isloopback(const saw_addr4_t *addr); */
#define saw_addr4_isloopback(source) _saw_addr4_isloopback(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isloopback(source) ((source)[0] == 127)
#define saw_addr4_islo(source) saw_addr4_isloopback(source)

/* extern bool saw_addr4_isany(const saw_addr4_t *addr); */
#define saw_addr4_isany(source) _saw_addr4_isany(((const saw_addr4_t*)(source)))
#define _saw_addr4_isany(source) (source->addr == saw_addr4_any.addr)

/* extern bool saw_addr4_isbroadcast(const saw_addr4_t *addr); */
#define saw_addr4_isbroadcast(source) _saw_addr4_isbroadcast(((const saw_addr4_t*)(source)))
#define _saw_addr4_isbroadcast(source) (source->addr == saw_addr4_broadcast.addr)
#define saw_addr4_isbc(source) saw_addr4_isbroadcast(source)

/* extern bool saw_addr4_isprivatenet(const saw_addr4_t *addr); */
#define saw_addr4_isprivatenet(source) _saw_addr4_isprivatenet(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isprivatenet(source) ( \
                                          ( \
                                            source[0] == 10 \
                                          ) \
                                          || \
                                          ( \
                                            source[0] == 172 && \
                                            source[1] >= 16 && source[1] <= 31 \
                                          ) \
                                          || \
                                          ( \
                                            source[0] == 192 && \
                                            source[1] == 168 \
                                          ) \
                                        )
#define saw_addr4_ispn(source) saw_addr4_isprivatenet(source)

/* extern bool saw_addr4_islinklocal(const saw_addr4_t *addr); */
#define saw_addr4_islinklocal(source) _saw_addr4_islinklocal(_saw_addr4_u8p_cast(source))
#define _saw_addr4_islinklocal(source) (source[0] == 169 && source[1] == 254)
#define saw_addr4_isll(source) saw_addr4_islinklocal(source)

/* extern bool saw_addr4_ismulticast(const saw_addr4_t *addr); */
#define saw_addr4_ismulticast(source) _saw_addr4_ismulticast(_saw_addr4_u8p_cast(source))
#define _saw_addr4_ismulticast(source) (source[0] >= 224 && source[0] <= 239)
#define saw_addr4_ismc(source) saw_addr4_ismulticast(source)

/* extern bool saw_addr4_isroutermc(const saw_addr4_t *addr); */
#define saw_addr4_isroutermc(source) _saw_addr4_isroutermc(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isroutermc(source) (source[0] == 224)
#define saw_addr4_isrmc(source) saw_addr4_isroutermc(source)

/* extern bool saw_addr4_isscopingmc(const saw_addr4_t *addr); */
#define saw_addr4_isscopingmc(source) _saw_addr4_isscopingmc(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isscopingmc(source) (source[0] == 239)
#define saw_addr4_issmc(source) saw_addr4_isscopingmc(source)

/* extern bool saw_addr4_is6to4relay(const saw_addr4_t *addr); */
#define saw_addr4_is6to4relay(source) _saw_addr4_is6to4relay(_saw_addr4_u8p_cast(source))
#define _saw_addr4_is6to4relay(source) (source[0] == 192 && source[1] == 88 && source[2] == 99)
#define saw_addr4_is6to4(source) saw_addr4_is6to4relay(source)

/* extern bool saw_addr4_isexample(const saw_addr4_t *addr); */
#define saw_addr4_isexample(source) _saw_addr4_isexample(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isexample(source) (source[0] == 192 && source[1] == 0 && source[2] == 2)
#define saw_addr4_isdoc(source) saw_addr4_isexample(source)

/* extern bool saw_addr4_isbenchmark(const saw_addr4_t *addr); */
#define saw_addr4_isbenchmark(source) _saw_addr4_isbenchmark(_saw_addr4_u8p_cast(source))
#define _saw_addr4_isbenchmark(source) (source[0] == 198 && (source[1] == 18 || source[1] == 19))
#define saw_addr4_isbm(source) saw_addr4_isbenchmark(source)

/* extern bool saw_addr4_issubnetmask(const saw_addr4_t *addr); */
#define saw_addr4_issubnetmask(source) _saw_addr4_issubnetmask(((const saw_addr4_t*)(source)))
#define _saw_addr4_issubnetmask(source) (!((source->addr + 1) & source->addr))
#define saw_addr4_issnm(source) saw_addr4_issubnetmask(source)

/* extern void saw_addr4_copy(const saw_addr4_t *first, saw_addr4_t *second); */
#define saw_addr4_copy(first, second) _saw_addr4_copy(((const saw_addr4_t*)(first)), ((saw_addr4_t*)(second)))
#define _saw_addr4_copy(first, second) (second->addr = first->addr)
#define saw_addr4_cpy(first, second) saw_addr4_copy((first), (second))

/* extern bool saw_addr4_compare(const saw_addr4_t *first, const saw_addr4_t *second); */
#define saw_addr4_compare(first, second) _saw_addr4_compare(((const saw_addr4_t*)(first)), ((const saw_addr4_t*)(second)))
#define _saw_addr4_compare(first, second) (first->addr == second->addr)
#define saw_addr4_cmp(first, second) saw_addr4_compare((first), (second))

/* extern void saw_addr4_to_local(const saw_addr4_t *ip, saw_addr4_local_t *pbuf); */
#define saw_addr4_to_local(ip, local) _saw_addr4_to_local(((const saw_addr4_t*)(ip)), ((saw_addr4_local_t*)(local)))
#define _saw_addr4_to_local(ip, local) (memset(local, 0, SAW_ADDR4_LOCSIZE), (local->s_addr = ip->addr))

/* extern void saw_addr4_from_local(const saw_addr4_local_t *local, saw_addr4_t *dest); */
#define saw_addr4_from_local(local, dest) _saw_addr4_from_local(((const saw_addr4_local_t*)(local)), ((saw_addr4_t*)(dest)))
#define _saw_addr4_from_local(local, dest) (dest->addr = local->s_addr)


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_addr4_h */

