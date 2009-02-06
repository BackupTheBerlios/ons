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

/* Miscellaneous declarations of FIP.
 * This contains global functions like the global init/deinit and some constant definitions.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_fip_misc_h
#define ONS_INCLUDED_fip_misc_h
ONS_EXTERN_C_BEGIN


/* Necessary configuration is provided by the libsaw headers. */
#include <saw/saw.h>

/* EINTR handler.
 * On Unixes most syscalls can return EINTR if they were interrupted by a signal. By default FIP retrys
 * the functions until it is not interrupted. This is the recommended strategy. But if you want FIP to return
 * after a given amount of attempts you can set this value.
 * A function returns FIP_E_SIGFLOOD in this case.
 * Set it to FIP_EINTR_ENDLESS if you want it to try until it succeeds.
 *
 * Be carefull. It's prefered to set it with fip_global_init().
 * Do not change this value during runtime of socket-functions in another thread!
 */
extern uint64_t fip_eintr_count;

/* Endless. */
#define FIP_EINTR_ENDLESS 0

/* Contains a single error code. */
typedef unsigned int fip_err_t;


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_fip_misc_h */

