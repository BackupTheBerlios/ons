/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. July 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. January 2009
 */

/* Error handling.
 * This file provides an error handling interface for the FIP library.
 */

#include <fip/misc.h>

#ifndef ONS_INCLUDED_fip_error_h
#define ONS_INCLUDED_fip_error_h
ONS_EXTERN_C_BEGIN


/* Error classes */
#define FIP_C_NONE              0x0000 /* No class. */
/* base classes */
#define FIP_C_GENERAL           0x0001
#define FIP_C_MISSING           0x0002
#define FIP_C_ADDRESS           0x0004
#define FIP_C_CONNECTING        0x0008
#define FIP_C_SHUTDOWN          0x0010
#define FIP_C_IO                0x0020
#define FIP_C_BINDING           0x0040
#define FIP_C_OPTION            0x0080
#define FIP_C_FILE              0x0100
/* additional classes */
#define FIP_C_CRITICAL          0x0200 /* There is no general rule which errors are "critical"
                                        * except that you should treat them separately/specially.
                                        */
#define FIP_C_CLOSED            0x0400 /* Socket is closed now. */
#define FIP_C_TEMPORARY         0x0800 /* Temporary failures (same constellation could succeed). */
#define FIP_C_FATAL             0x1000 /* Constellation which will never succeed in this process. */

typedef struct fip_einfo_t {
    unsigned int eclass;
    const char *emsg;
} fip_einfo_t;

/* Error code information */
extern const fip_einfo_t const fip_error_info[ONS_E_LAST];

/* Returns the classes an error code is member of. */
#define FIP_ECLASS(ecode) (fip_error_info[(ecode)].eclass)

/* Returns 1 if an ERROR CODE is part of a class, otherwise 0. */
#define FIP_EISOF(ecode, eclass) (FIP_ECLASS(ecode) & eclass)

/* Takes an internal system dependent error code and returns the
 * FIP equivalent. Not every FIP error has one single equivalent.
 * It is kinda more complicated.
 *
 * _file: recognizes only filesystem related error codes.
 */
extern ons_err_t fip_mkerrno(signed int err);
extern ons_err_t fip_mkerrno_file(signed int err);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_error_h */

