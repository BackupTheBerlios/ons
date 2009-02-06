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

/* Helper functions for file domain sockets.
 * This type is also known as PF_UNIX/PF_FILE/PF_LOCAL. Although PF_UNIX is the
 * most common type it is an old BSD style name. PF_LOCAL is preferred here due to it's
 * OS-independency and because it is up to date.
 *
 * The namespaces are saw_addrlo_* and SAW_ADDRLO_*.
 * "loaddr" stands for "Local to host (pipes and file-domain) Address" and
 * is referred here as "file address" or similar.
 * There is some overlapping with the IPv4/6 "local" addresses which refer to the
 * IPv4/6 address structure of the system. It should be clear in the context, though.
 *
 * A file address is a single filename. It refers to a file in your file system and it
 * is zero terminated.  The maximal length is SAW_ADDRLO_STRLEN.
 * If you want to create a file in the abstract file system the first byte of the string must
 * be a zero-byte. The rest of the string is the filename. Notice that the rest is NOT zero
 * terminated, thus, it is SAW_ADDRLO_STRLEN - 1 bytes long.
 * Abstract file names are not supported on all platforms. See the documentation and try to
 * avoid abstract file names.
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_addrlo_h
#define ONS_INCLUDED_saw_addrlo_h
ONS_EXTERN_C_BEGIN


#ifdef ONS_CONF_HAVE_UNIX_SOCKETS
    #ifdef ONS_CONF_HAVE_SYS_SOCKET_H
        #include <sys/socket.h>
    #endif
    #ifdef ONS_CONF_HAVE_SYS_UN_H
        #include <sys/un.h>
    #endif
#endif

#include <stdlib.h>
#include <string.h>

#ifdef AF_UNIX
    #define SAW_ADDRLO_FAM ((signed int)AF_UNIX)
#elif defined(AF_LOCAL)
    #define SAW_ADDRLO_FAM ((signed int)AF_LOCAL)
#elif defined(AF_FILE)
    #define SAW_ADDRLO_FAM ((signed int)AF_FILE)
#else
    #define SAW_ADDRLO_FAM (0)
#endif

#ifdef PF_UNIX
    #define SAW_ADDRLO_DOM ((signed int)PF_UNIX)
#elif defined(PF_LOCAL)
    #define SAW_ADDRLO_DOM ((signed int)PF_LOCAL)
#elif defined(PF_FILE)
    #define SAW_ADDRLO_DOM ((signed int)PF_FILE)
#else
    #define SAW_ADDRLO_DOM SAW_ADDRLO_FAM
#endif

#ifndef ONS_CONF_HAVE_UNIX_SOCKETS
    /* 108 is the common limit so we force it on systems without unix sockets. */
    #define SAW_ADDRLO_STRLEN ((unsigned int)108)
#else
    #define SAW_ADDRLO_STRLEN ONS_CONF_PATH_SIZE
#endif

#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(push)
    #pragma pack(1)
#endif
struct saw_addrlo_t {
    uint16_t family;
    char addr[SAW_ADDRLO_STRLEN];
} ONS_ATTR_PACK;
#ifdef ONS_CONF_HAVE_PRAGMA_PACK
    #pragma pack(pop)
#endif

typedef struct saw_addrlo_t saw_addrlo_t;
#define SAW_ADDRLO_SIZE ((unsigned int)sizeof(saw_addrlo_t))

#ifndef ONS_CONF_HAVE_UNIX_SOCKETS
    /* If we have no local equivalent we specify our structure as local structure, too. */
    typedef saw_addrlo_t saw_addrlo_local_t;
#else
    typedef struct sockaddr_un saw_addrlo_local_t;
#endif
#define SAW_ADDRLO_LOCSIZE ((unsigned int)sizeof(saw_addrlo_local_t))

/* Sets the address string. The name is cropped at the SAW_ADDRLO_STRLEN'th offset.
 * This function does not check whether your string ends with \0. Please check yourself
 * whether your name is valid.
 * A name is valid if it ends on '\0' or if it is an abstract filename which is described above in the
 * introduction of this file (see also documentation).
 * \namelen contains the size of \name with the possible terminating zero character.
 * ** extern void saw_addrlo_set(struct saw_addrlo_t *p_addr, const char *name, unsigned int namelen);
 */
#define saw_addrlo_set(p_addr, name, namelen) _saw_addrlo_set(((struct saw_addrlo_t*)(p_addr)), ((const char*)(name)), ((unsigned int)(namelen)))
#define _saw_addrlo_set(p_addr, name, namelen) ( \
                                                  memcpy( \
                                                            p_addr->addr, \
                                                            name, \
                                                            (namelen > SAW_ADDRLO_STRLEN)?SAW_ADDRLO_STRLEN:namelen \
                                                        ), \
                                                  saw_addrlo_setfam(p_addr) \
                                               )

/* Sets the SAW_ADDRLO_FAM in \p_addr. */
/* extern void saw_addrlo_setfam(struct saw_addrlo_t *p_addr); */
#define saw_addrlo_setfam(p_addr) _saw_addrlo_setfam(((struct saw_addrlo_t*)(p_addr)))
#define _saw_addrlo_setfam(p_addr) (p_addr->family = SAW_ADDRLO_FAM)

/* Copys \first into \second. */
/* extern void saw_addrlo_copy(const saw_addrlo_t *first, saw_addrlo_t *second); */
#define saw_addrlo_copy(first, second) _saw_addrlo_copy(((const saw_addrlo_t*)(first)), ((saw_addrlo_t*)(second)))
#define _saw_addrlo_copy(first, second) (memcpy(second, first, SAW_ADDRLO_SIZE))
#define saw_addrlo_cpy(first, second) saw_addrlo_copy((first), (second))

/* Returns 1 if both addresses are equal. If the first character of the filename is \0 the whole
 * filename is compared, otherwise a single stringcomparison is performed.
 */
/* extern bool saw_addrlo_compare(const saw_addrlo_t *first, const saw_addrlo_t *second); */
#define saw_addrlo_compare(first, second) _saw_addrlo_compare(((const saw_addrlo_t*)(first)), ((const saw_addrlo_t*)(second)))
#define _saw_addrlo_compare(first, second) ((first->addr[0] == '\0')? \
                                                (first->family == second->family && 0 == memcmp(first, second, SAW_ADDRLO_SIZE)) \
                                                :(first->family == second->family && \
                                                  0 == strncmp(first->addr, second->addr, SAW_ADDRLO_STRLEN)) \
                                           )
#define saw_addrlo_cmp(first, second) saw_addrlo_compare((first), (second))

/* sun_len wrapper used in converters below. */
#ifdef ONS_CONF_HAVE_SUN_LEN
    #define _SAW_SUN_LEN(x, y) (y->sun_len = x)
#else
    #define _SAW_SUN_LEN(x, y) (0)
#endif

#ifndef ONS_CONF_HAVE_UNIX_SOCKETS
    /* On windows there is no such local address, thus, we use the same struct and a memcpy is enough. */
    #define saw_addrlo_to_local(p_addr, local) (memcpy((void*)local, (void*)p_addr, SAW_ADDRLO_SIZE))
    #define saw_addrlo_from_local(local, p_addr) (memcpy((void*)p_addr, (void*)local, SAW_ADDRLO_SIZE))
#else
    /* Converts a SAW file address into a local file structure format. */
    /* extern void saw_addrlo_to_local(const struct saw_addrlo_t *p_addr, saw_addrlo_local_t *local); */
    #define saw_addrlo_to_local(p_addr, local) _saw_addrlo_to_local(((const struct saw_addrlo_t*)(p_addr)), ((saw_addrlo_local_t*)(local)))
    #define _saw_addrlo_to_local(p_addr, local) ((void)( \
                                                    (memset(local, 0, SAW_ADDRLO_LOCSIZE)), \
                                                    (local->sun_family = SAW_ADDRLO_FAM), \
                                                    (_SAW_SUN_LEN(SAW_ADDRLO_LOCSIZE, local)), \
                                                    (memcpy(local->sun_path, p_addr->addr, SAW_ADDRLO_STRLEN)) \
                                                ))

    /* Converts a local address into the SAW file address structure format. */
    /* extern void saw_addrlo_from_local(const saw_addrlo_local_t *local, struct saw_addrlo_t *p_addr); */
    #define saw_addrlo_from_local(local, p_addr) _saw_addrlo_from_local(((const saw_addrlo_local_t*)(local)), ((struct saw_addrlo_t*)(p_addr)))
    #define _saw_addrlo_from_local(local, p_addr) ( \
                                                    (p_addr->family = SAW_ADDRLO_FAM), \
                                                    memcpy(p_addr->addr, local->sun_path, SAW_ADDRLO_STRLEN) \
                                                 )
#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_addrlo_h */

