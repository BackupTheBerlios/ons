/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 8. May 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Global wrapper for all addresses in SAW.
 * Please cast your pointers from addrx/addrlo addresses to this type and pass
 * it to any function which will automatically detect the address type by the
 * ->raw.family member.
 *
 * If any function needs a buffer where to store a returned address, you should
 * pass a new saw_addr buffer unless you know which address will be returned
 * and you can pass smaller buffers.
 *
 * Namespaces are: saw_addr_* and SAW_ADDR_*
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_addr_h
#define ONS_INCLUDED_saw_addr_h
ONS_EXTERN_C_BEGIN


#ifdef ONS_CONF_HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif

#include <saw/addr4.h>      /* IPv4 address manipulating header */
#include <saw/addr6.h>      /* IPv6 address manipulating header */
#include <saw/addrx.h>      /* Wrapper for IPv4 and IPv6 addresses */
#include <saw/addrlo.h>     /* Local address manipulating header */

/* Defines the local datatype used to store addresses. */
typedef struct sockaddr saw_addr_local_t;

/* Defines the size of the biggest local address structure beeing used in saw_addr (used as size for "saw_addr_local"). */
#define SAW_ADDR_LOCSIZE ((SAW_ADDRX_LOCSIZE > SAW_ADDRLO_LOCSIZE)?SAW_ADDRX_LOCSIZE:SAW_ADDRLO_LOCSIZE)

/* Maximal size of a saw_addr structure.
 * KEEP THIS EQUIVALENT TO sizeof(saw_addr_t) !
 * (Can't use "sizeof(saw_addr_t)", because this constant is used inside saw_addr_t.)
 */
#define SAW_ADDR_SIZE ((SAW_ADDRX_SIZE > SAW_ADDRLO_SIZE)?SAW_ADDRX_SIZE:SAW_ADDRLO_SIZE)

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(push)
    #pragma pack(1)
#endif
union saw_addr_t {
    struct saw_addr_raw_t {
        uint16_t family;
        uint8_t raw[SAW_ADDR_SIZE - 2];
    } ONS_ATTR_PACK raw;
    saw_addrx_t in;
    saw_addrlo_t lo;
} ONS_ATTR_PACK;
typedef union saw_addr_t saw_addr_t;
#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(pop)
#endif

/* Copys address \first to \buffer second. */
/* extern void saw_addr_copy(const saw_addr_t *first, saw_addr_t *second); */
#define saw_addr_copy(first, second) _saw_addr_copy(((const saw_addr_t*)(first)), ((saw_addr_t*)(second)))
#define _saw_addr_copy(first, second) (memcpy(second, first, SAW_ADDR_SIZE))
#define saw_addr_cpy(first, second) saw_addr_copy((first), (second))

/* Returns 1 if address \first is the same as \second.
 * If \family is SAW_ADDRLO_FAM, SAW_ADDR4_FAM or SAW_ADDR6_FAM, their *_compare macros are used and
 * the padding bytes in the union are not compared! But if we do not know the family we use
 * memcmp on the whole union.
 * If the addresses do not match, 0 is returned.
 */
/* extern bool saw_addr_compare(const saw_addr_t *first, const saw_addr_t *second); */
#define saw_addr_compare(first, second) _saw_addr_compare(((const saw_addr_t*)(first)), ((const saw_addr_t*)(second)))
#define _saw_addr_compare(first, second) ( \
                                            (first->raw.family != second->raw.family)?0: \
                                            (\
                                                (first->raw.family == SAW_ADDRLO_FAM)? \
                                                saw_addrlo_compare(&first->lo, &second->lo) \
                                                :( \
                                                    (first->raw.family == SAW_ADDR4_FAM || first->raw.family == SAW_ADDR6_FAM)? \
                                                    saw_addrx_compare(&first->in, &second->in) \
                                                    :(0 == memcmp(first->raw.raw, second->raw.raw, sizeof(saw_addr_t))) \
                                                ) \
                                            )\
                                          )
#define saw_addr_cmp(first, second) saw_addr_compare((first), (second))

/* Converts a saw_addr into the local address structure format.
 * ** extern void saw_addr_to_local(const saw_addr_t *source, saw_addr_local_t *dest);
 */
#define saw_addr_to_local(source, dest) _saw_addr_to_local(((const saw_addr_t*)(source)), ((saw_addr_local_t*)(dest)))
#define _saw_addr_to_local(source, dest) ( \
                                            (source->raw.family == SAW_ADDRLO_FAM)? \
                                                saw_addrlo_to_local(source, dest): \
                                                saw_addrx_to_local(source, dest) \
                                         )

/* Converts a local address into the SAW address structure format.
 * Return 0 if the local address is not known. 1 on success.
 * ** extern bool saw_addr_from_local(const saw_addr_local_t *source, saw_addr_t *dest);
 */
#define saw_addr_from_local(source, dest) _saw_addr_from_local(((const saw_addr_local_t*)(source)), ((saw_addr_t*)(dest)))
#define _saw_addr_from_local(source, dest) ( \
                                              (source->sa_family == SAW_ADDRLO_FAM)? \
                                                  (saw_addrlo_from_local(source, dest), 1): \
                                                  (saw_addrx_from_local(source, dest)) \
                                           )

/* Returns the size of a local structure.
 * Returns 0 if the local structure is unknown.
 * ** extern unsigned int saw_addr_locsize(const saw_addr_local_t *source);
 */
#define saw_addr_locsize(source) _saw_addr_locsize(((const saw_addr_local_t*)(source)))
#define _saw_addr_locsize(source) ( \
                                    (source->sa_family == SAW_ADDR4_FAM)? \
                                        SAW_ADDRX_LOCSIZE4: \
                                        ( \
                                            (source->sa_family == SAW_ADDR6_FAM)? \
                                                SAW_ADDRX_LOCSIZE6: \
                                                ( \
                                                    (source->sa_family == SAW_ADDRLO_FAM)? \
                                                    SAW_ADDRLO_LOCSIZE: \
                                                    0 \
                                                ) \
                                        ) \
                                  )


ONS_EXTERN_C_END
#endif

