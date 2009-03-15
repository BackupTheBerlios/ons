/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 15. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* WINDOWS specific API description
 * We describe the WIN32 and WIN64 API here. We try to support
 * all aspects of windows.
 * However, WIN16 support is not included.
 *
 * David Herrmann -- 2009/03/15
 */


#ifndef ONS_INCLUDED_ons_linux_h
#define ONS_INCLUDED_ons_linux_h
#ifdef __cplusplus
extern 'C' {
#endif


/* API: System
 * This API is required and must be supported on all systems.
 */
#define ONS_SYSTEM


/* API: String
 * This API is available on windows machines, however, strnlen is not
 * available.
 */
#define ONS_STRING
/* #define ONS_STRING_STRNLEN */


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_linux_h */

