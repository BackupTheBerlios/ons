/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 3. July 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Network Interfaces
 * This file provides an API to handle the network interfaces in
 * a unique manner.
 */

#include <fip/misc.h>

#ifndef ONS_INCLUDED_fip_if_h
#define ONS_INCLUDED_fip_if_h
ONS_EXTERN_C_BEGIN


/* Interfaces will be implemented in FIP >1.0.0 */

/*#include <net/if.h>*/         /* IFNAMSIZ declaration */
/* Defines maixmal length of a devicename. */
/*#ifdef IF_NAMESIZE
    #define FIP_DEV_STRLEN (IF_NAMESIZE)
#elif defined(IFNAMSIZ)
    #define FIP_DEV_STRLEN (IFNAMSIZ)
#else
    #define FIP_DEV_STRLEN 16
    #warning "IF_NAMESIZE nor IFNAMSIZ are found on your machine, defining it as 16 (default)."
#endif

extern unsigned int fip_bindev(fip_err *err, fip_fd fd, const char *device);*/


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_if_h */

