/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 23. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 7. February 2009
 */

/* ONS main functions.
 * Contains the main functionality.
 */

#include <ons/misc.h>

#ifndef ONS_INCLUDED_ons_main_h
#define ONS_INCLUDED_ons_main_h
ONS_EXTERN_C_BEGIN


#include <ons/sys.h>
#include <ons/err.h>

/* Options which can be passed to ons_(de)init(). */
enum {
    ONS_INIT_WSA,   /* init: Calls the WSA initialization routines on Windows. On other machines it has no effect.
                     * deinit: Calls the WSA deinitialization routines on Windows. On other machines it has no effect.
                     */
    #define ONS_INIT_WSA ONS_BIT(ONS_INIT_WSA)
};

/* Initialize the core library.
 * ons_init() returns ONS_E_SUCCESS on success, otherwise an error code is returned.
 * ons_deinit() always succeeds.
 * The \opts argument specifys the operations which should be done by the
 * functions.
 * Both functions are not threadsafe. Please call them once before/after
 * having created/deleted all other threads which could also call them.
 */
extern ons_err_t ons_init(ons_bitset8_t opts);
extern void ons_deinit(ons_bitset8_t opts);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_ons_main_h */

