/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 19. April 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 17. January 2009
 */

/* Wrapper header for addr4.h and addr6.h.
 * This header provides an interface to handle all IP socket addresses
 * of this library in one easy way.
 *
 * If any operation which operates on saw_addrx_t structures returns 0, the
 * family/addr fields of the saw_addrx_t structure are undefinied. To solve this retry the operation
 * with correct parameters or reset it with saw_addrx_default() or try to use other functions.
 * "void" functions never fail!
 *
 * Namespaces: saw_addrx_* and SAW_ADDRX_*
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_addrx_h
#define ONS_INCLUDED_saw_addrx_h
ONS_EXTERN_C_BEGIN


#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif

#include <string.h>
#include <saw/addr4.h>
#include <saw/addr6.h>

/* Defines a port number used as random port number. */
#define SAW_ADDRX_ANYPORT ((uint16_t)0)

#ifdef ONS_CONF_NO_IPV6
    #define SAW_ADDRX_STRLEN SAW_ADDR4_STRLEN
#else
    #define SAW_ADDRX_STRLEN ((SAW_ADDR6_STRLEN > SAW_ADDR4_STRLEN)?SAW_ADDR6_STRLEN:SAW_ADDR4_STRLEN)
#endif

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(push)
    #pragma pack(1)
#endif
struct saw_addrx_t {
    uint16_t family;
    saw_port_t port;
    union saw_addrx_raw_t {
        saw_addr4_t in4;
#ifdef ONS_CONF_NO_IPV6
        uint8_t raw[SAW_ADDR4_SIZE];
#else
        saw_addr6_t in6;
        uint8_t raw[(SAW_ADDR6_SIZE > SAW_ADDR4_SIZE)?SAW_ADDR6_SIZE:SAW_ADDR4_SIZE];
#endif
    } ONS_ATTR_PACK addr;
} ONS_ATTR_PACK;
#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(pop)
#endif

typedef struct saw_addrx_t saw_addrx_t;
#define SAW_ADDRX_SIZE ((unsigned int)sizeof(saw_addrx_t))

typedef struct sockaddr saw_addrx_local_t;
typedef struct sockaddr_in saw_addrx_local4_t;
#define SAW_ADDRX_LOCSIZE4 ((unsigned int)sizeof(saw_addrx_local4_t))
#define SAW_ADDRX_LOCSIZE SAW_ADDRX_LOCSIZE4
#ifndef ONS_CONF_NO_IPV6
    typedef struct sockaddr_in6 saw_addrx_local6_t;
    #define SAW_ADDRX_LOCSIZE6 ((unsigned int)sizeof(saw_addrx_local6_t))
    #undef SAW_ADDRX_LOCSIZE
    #define SAW_ADDRX_LOCSIZE ((SAW_ADDRX_LOCSIZE4 > SAW_ADDRX_LOCSIZE6)?SAW_ADDRX_LOCSIZE4:SAW_ADDRX_LOCSIZE6)
#endif

/* Sets the port to \p_port. */
/* extern void saw_addrx_setport(saw_addrx_t *p_addr, saw_port_t p_port); */
#define saw_addrx_setport(p_addr, p_port) _saw_addrx_setport(((saw_addrx_t*)(p_addr)), ((saw_port_t)(p_port)))
#define _saw_addrx_setport(p_addr, p_port) (p_addr->port = saw_hton16(p_port))

/* Returns the port of \p_addr. */
/* extern saw_port_t saw_addrx_getport(const saw_addrx_t *p_addr); */
#define saw_addrx_getport(p_addr) _saw_addrx_getport(((const saw_addrx_t*)(p_addr)))
#define _saw_addrx_getport(p_addr) (saw_ntoh16(p_addr->port))

/* Returns 1 if \p_addr is of type SAW_ADDR4_FAM. */
/* extern void saw_addrx_is4(const saw_addrx_t *p_addr); */
#define saw_addrx_is4(p_addr) _saw_addrx_is4(((const saw_addrx_t*)(p_addr)))
#define _saw_addrx_is4(p_addr) (p_addr->family == SAW_ADDR4_FAM)

/* Returns 1 if \p_addr is of type SAW_ADDR6_FAM. */
/* extern void saw_addrx_is6(const saw_addrx_t *p_addr); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_is6(p_addr) _saw_addrx_is6(((const saw_addrx_t*)(p_addr)))
    #define _saw_addrx_is6(p_addr) (p_addr->family == SAW_ADDR6_FAM)
#endif

/* Copys address \first to \buffer second. */
/* extern void saw_addrx_copy(const saw_addrx_t *first, saw_addrx_t *second); */
#define saw_addrx_copy(first, second) _saw_addrx_copy(((const saw_addrx_t*)(first)), ((saw_addrx_t*)(second)))
#define _saw_addrx_copy(first, second) (memcpy(second, first, SAW_ADDRX_SIZE))
#define saw_addrx_cpy(first, second) saw_addrx_copy((first), (second))

/* Returns 1 if address \first is the same as \second.
 * If \family is SAW_ADDR4_FAM or SAW_ADDR6_FAM, their *_compare macros are used and
 * the padding bytes in the union are not compared! But if we do not know the family we use
 * memcmp on the whole union.
 * If the addresses do not match, 0 is returned.
 */
/* extern bool saw_addrx_compare(const saw_addrx_t *first, const saw_addrx_t *second); */
#define saw_addrx_compare(first, second) _saw_addrx_compare(((const saw_addrx_t*)(first)), ((const saw_addrx_t*)(second)))
#define _saw_addrx_compare(first, second) ( \
                                            (first->family != second->family)?0: \
                                            (\
                                                (first->port != second->port)?0: \
                                                (\
                                                    (first->family == SAW_ADDR4_FAM)? \
                                                    saw_addr4_compare(&first->addr.in4, &second->addr.in4) \
                                                    :( \
                                                        (first->family == SAW_ADDR6_FAM)? \
                                                        saw_addr6_compare(&first->addr.in6, &second->addr.in6) \
                                                        :(0 == memcmp(first->addr.raw, second->addr.raw, sizeof(union saw_addrx_raw_t))) \
                                                    ) \
                                                )\
                                            )\
                                          )
#define saw_addrx_cmp(first, second) saw_addrx_compare((first), (second))

/* Creates an address out of a string. Return 1 on success, otherwise 0. */
/* extern bool saw_addrx_pton(const char *p_addr, saw_addrx_t *out); */
#ifdef ONS_CONF_NO_IPV6
    #define saw_addrx_pton(p_addr, out) saw_addrx_pton4((p_addr), (out))
#else
    #define saw_addrx_pton(p_addr, out) _saw_addrx_pton(((const char*)(p_addr)), ((saw_addrx_t*)(out)))
    #define _saw_addrx_pton(p_addr, out) ( \
                                            saw_addrx_pton4(p_addr, out)? \
                                                1: \
                                                saw_addrx_pton6(p_addr, out) \
                                         )
#endif

/* Creates an IPv4 address out of a string. Return 1 on success, otherwise 0. */
/* extern bool saw_addrx_pton4(const char *p_addr, saw_addrx_t *out); */
#define saw_addrx_pton4(p_addr, out) _saw_addrx_pton4(((const char*)(p_addr)), ((saw_addrx_t*)(out)))
#define _saw_addrx_pton4(p_addr, out) ( \
                                        saw_addr4_pton(p_addr, &out->addr.in4)? \
                                            ((out->family = SAW_ADDR4_FAM), 1): \
                                            0 \
                                      )

/* Creates an IPv6 address out of a string. Return 1 on success, otherwise 0. */
/* extern bool saw_addrx_pton6(const char *p_addr, saw_addrx_t *out); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_pton6(p_addr, out) _saw_addrx_pton6(((const char*)(p_addr)), ((saw_addrx_t*)(out)))
    #define _saw_addrx_pton6(p_addr, out) ( \
                                            saw_addr6_pton(p_addr, &out->addr.in6)? \
                                                ((out->family = SAW_ADDR6_FAM), 1): \
                                                0 \
                                          )
#endif

/* Creates a string out of an address. */
/* extern void saw_addr_ntop(const saw_addrx_t *p_addr, char *out); */
#ifdef ONS_CONF_NO_IPV6
    #define saw_addrx_ntop(p_addr, out) _saw_addrx_ntop(((const saw_addrx_t*)(p_addr)), ((char*)(out)))
    #define _saw_addrx_ntop(p_addr, out) saw_addr4_ntop(p_addr->addr.in4, out)
#else
    #define saw_addrx_ntop(p_addr, out) _saw_addrx_ntop(((const saw_addrx_t*)(p_addr)), ((char*)(out)))
    #define _saw_addrx_ntop(p_addr, out) ( \
                                            saw_addrx_is4(p_addr)? \
                                                saw_addr4_ntop(&p_addr->addr.in4, out): \
                                                saw_addr6_ntop(&p_addr->addr.in6, out) \
                                         )
#endif

/* see below */
#ifdef ONS_CONF_NO_IPV6
    #define saw_addrx_default(x) saw_addrx_default4(x)
#else
    #define saw_addrx_default(x) saw_addrx_default6(x)
#endif

/* Initializes the address with SAW_ADDR4_ANY and SAW_ADDRX_ANYPORT. */
/* extern void saw_addrx_default4(saw_addrx_t *p_addr); */
#define saw_addrx_default4(p_addr) _saw_addrx_default4(((saw_addrx_t*)(p_addr)))
#define _saw_addrx_default4(p_addr) ((void)( \
                                        saw_addrx_set_ipv4(p_addr, &saw_addr4_any), \
                                        (p_addr->port = SAW_ADDRX_ANYPORT) \
                                    ))

/* Initializes the address with SAW_ADDR6_ANY and SAW_ADDRX_ANYPORT. */
/* extern void saw_addrx_default6(saw_addrx_t *p_addr); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_default6(x) _saw_addrx_default6(((saw_addrx_t*)(x)))
    #define _saw_addrx_default6(x) ((void)( \
                                    saw_addrx_set_ipv6(x, &saw_addr6_any), \
                                    (x->port = SAW_ADDRX_ANYPORT) \
                                   ))
#endif

/* Sets an IPv4 address into the structure. */
/* extern void saw_addrx_set_ipv4(saw_addrx_t *p_addr, const saw_addr4_t *in); */
#define saw_addrx_set_ipv4(x, y) _saw_addrx_set_ipv4(((saw_addrx_t*)(x)), ((const saw_addr4_t*)(y)))
#define _saw_addrx_set_ipv4(x, y) ((void)( \
                                    memcpy(&x->addr.in4, y, SAW_ADDR4_SIZE), \
                                    (x->family = SAW_ADDR4_FAM) \
                                  ))

/* Sets an IPv6 address into the structure. */
/* extern void saw_addrx_set_ipv6(saw_addrx_t *p_addr, const saw_addr6_t *in); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_set_ipv6(x, y) _saw_addrx_set_ipv6(((saw_addrx_t*)(x)), ((const saw_addr6_t*)(y)))
    #define _saw_addrx_set_ipv6(x, y) ((void)( \
                                        memcpy(&x->addr.in6, y, SAW_ADDR6_SIZE), \
                                        (x->family = SAW_ADDR6_FAM) \
                                      ))
#endif

/* Converts the structure into a local structure. */
/* extern void saw_addrx_to_local(const saw_addrx_t *p_addr, saw_addrx_local_t *dest); */
#ifdef ONS_CONF_NO_IPV6
    #define saw_addrx_to_local(x, y) ((void)(saw_addrx_to_local4((x), (y))))
#else
    #define saw_addrx_to_local(x, y) _saw_addrx_to_local(((const saw_addrx_t*)(x)), ((saw_addrx_local_t*)(y)))
    #define _saw_addrx_to_local(x, y) ( \
                                        saw_addrx_is4(x)? \
                                            ((void)(saw_addrx_to_local4(x, y))): \
                                            ((void)(saw_addrx_to_local6(x, y))) \
                                      )
#endif

/* sin(6)_len wrapper used in converters below. */
#ifdef ONS_CONF_HAVE_SIN_LEN
    #define _SAW_SIN_LEN(x, y) (y->sin_len = x)
#else
    #define _SAW_SIN_LEN(x, y) (0)
#endif
#ifdef ONS_CONF_HAVE_SIN6_LEN
    #define _SAW_SIN6_LEN(x, y) (x->sin6_len = y)
#else
    #define _SAW_SIN6_LEN(x, y) (0)
#endif

/* Converts the structure into a local IPv4 structure. */
/* extern void saw_addrx_to_local4(const saw_addrx_t *addr, saw_addrx_local4_t *dest); */
#define saw_addrx_to_local4(x, y) _saw_addrx_to_local4(((const saw_addrx_t*)(x)), ((saw_addrx_local4_t*)(y)))
#define _saw_addrx_to_local4(x, y) ( \
                                    memset(y, 0, SAW_ADDRX_LOCSIZE4), \
                                    (y->sin_family = SAW_ADDR4_FAM), \
                                    (y->sin_port = x->port), \
                                    (_SAW_SIN_LEN(SAW_ADDRX_LOCSIZE4, y)), \
                                    saw_addr4_to_local(&x->addr.in4, &y->sin_addr) \
                                   )

/* Converts the structure into a local IPv6 structure. */
/* extern void saw_addrx_to_local6(const saw_addrx_t *addr, saw_addrx_local6_t *dest); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_to_local6(x, y) _saw_addrx_to_local6(((const saw_addrx_t*)(x)), ((saw_addrx_local6_t*)(y)))
    #define _saw_addrx_to_local6(x, y) ( \
                                        memset(y, 0, SAW_ADDRX_LOCSIZE6), \
                                        (y->sin6_family = SAW_ADDR6_FAM), \
                                        (y->sin6_port = x->port), \
                                        (_SAW_SIN6_LEN(SAW_ADDRX_LOCSIZE6, y)), \
                                        saw_addr6_to_local(&x->addr.in6, &y->sin6_addr) \
                                       )
#endif

/* Creates an address out of a local address structure. Returns 1 on success and 0 if the local type is not known. */
/* extern bool saw_addrx_from_local(const saw_addrx_local_t *dest, saw_addrx_t *addr); */
#ifdef ONS_CONF_NO_IPV6
    #define saw_addrx_from_local(x, y) _saw_addrx_from_local(((const saw_addrx_local_t*)(x)), ((saw_addrx_t*)(y)))
    #define _saw_addrx_from_local(x, y) ( \
                                            (x->sa_family == SAW_ADDR4_FAM)? \
                                                (saw_addrx_from_local4(x, y), 1): \
                                                0 \
                                        )
#else
    #define saw_addrx_from_local(x, y) _saw_addrx_from_local(((const saw_addrx_local_t*)(x)), ((saw_addrx_t*)(y)))
    #define _saw_addrx_from_local(x, y) ( \
                                            (x->sa_family == SAW_ADDR4_FAM)? \
                                                (saw_addrx_from_local4(x, y), 1): \
                                                ( \
                                                    (x->sa_family == SAW_ADDR6_FAM)? \
                                                        (saw_addrx_from_local6(x, y), 1): \
                                                        0 \
                                                ) \
                                        )
#endif

/* Creates an address out of a local IPv4 address structure. Returns 1 on success and 0 if the local type is not known. */
/* extern void saw_addrx_from_local4(const saw_addrx_local4_t *dest, saw_addrx_t *addr); */
#define saw_addrx_from_local4(x, y) _saw_addrx_from_local4(((const saw_addrx_local4_t*)(x)), ((saw_addrx_t*)(y)))
#define _saw_addrx_from_local4(x, y) ( \
                                      memset(y, 0, SAW_ADDRX_SIZE), \
                                      (y->family = SAW_ADDR4_FAM), \
                                      (y->port = x->sin_port), \
                                      saw_addr4_from_local(&x->sin_addr, &y->addr.in4) \
                                     )

/* Creates an address out of a local IPv6 address structure. Returns 1 on success and 0 if the local type is not known. */
/* extern void saw_addrx_from_local6(const saw_addrx_local6_t *dest, saw_addrx_t *addr); */
#ifndef ONS_CONF_NO_IPV6
    #define saw_addrx_from_local6(x, y) _saw_addrx_from_local6(((const saw_addrx_local6_t*)(x)), ((saw_addrx_t*)(y)))
    #define _saw_addrx_from_local6(x, y) ( \
                                          memset(y, 0, SAW_ADDRX_SIZE), \
                                          (y->family = SAW_ADDR6_FAM), \
                                          (y->port = x->sin6_port), \
                                          saw_addr6_from_local(&x->sin6_addr, &y->addr.in6) \
                                         )
#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_addrx_h */

