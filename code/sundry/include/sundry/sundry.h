/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 25. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. May 2009
 */

/* Miscellaneous
 * Several sundry definitions and macros.
 */


#ifndef SUNDRY_INCLUDED_sundry_sundry_h
#define SUNDRY_INCLUDED_sundry_sundry_h
#ifdef __cplusplus
extern 'C' {
#endif


/* NULL is used so often. Include it here. */
#include <stddef.h>


/* To make all headers available in C++ they need to define "extern 'C'".
 * This macros defines "extern 'C'" if the compiler is a C++ compiler.
 * At the end of the header you must close "extern 'C'".
 *
 * SUNDRY_EXTERN_C: Defined as "extern 'C'" if __cplusplus is defined.
 * SUNDRY_EXTERN_C_BEGIN: Defined as "extern 'C' {" if __cplusplus is defined.
 * SUNDRY_EXTERN_C_END: Defined as "}" if __cplusplus is defined.
 */
#ifdef __cplusplus
    #define SUNDRY_EXTERN_C extern 'C'
    #define SUNDRY_EXTERN_C_BEGIN ONS_EXTERN_C {
    #define SUNDRY_EXTERN_C_END }
#else
    #define SUNDRY_EXTERN_C
    #define SUNDRY_EXTERN_C_BEGIN
    #define SUNDRY_EXTERN_C_END
#endif


/* Include the required headers. */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


/* This function prints an error message and exits the application.
 * If \format is NULL, a default message is printed.
 */
static void sundry_verr(const char *format, va_list list) {
    if(format != NULL) vfprintf(stderr, format, list);
    else fprintf(stderr, "Discovered a fatal error. No error message specified; Exiting...\n");
    abort();
}
static void sundry_ferr(const char *format, ...) {
    va_list list;
    va_start(list, format);
    sundry_verr(format, list);
    va_end(list);
    abort();
}


/* This function prints an error message and returns.
 * If \format is NULL, a default message is printed.
 */
static void sundry_vdebug(const char *format, va_list list) {
    if(format != NULL) vfprintf(stderr, format, list);
    else fprintf(stderr, "Discovered a debug error. No debug message specified; Ignoring...\n");
}
static void sundry_fdebug(const char *format, ...) {
    va_list list;

    va_start(list, format);
    sundry_vdebug(format, list);
    va_end(list);
}


/* SUNDRY_(F)ASSERT: This macro is equal to ISO-c89 assert() but does not depend on NDEBUG but on ONS_DEBUG_MODE and
 *                   requires a second parameter which must be a string defining an error message.
 * SUNDRY_DEBUG: This macro prints a message to stdout that a bug occurred and should be
 *               reported when ONS_DEBUG_MODE is defined and greater or equal to 3.
 * SUNDRY_ABORT: Prints a message and raises an exception. This often causes a core to be dumped.
 *
 * Because ONS_DEBUG_MODE is not defined in any header included here, you have to define it before using this macro, otherwise
 * the functions will always do nothing which can also be the expected behaviour.
 * source files which are part of ONS should include "machine.h" which defines ONS_DEBUG_MODE.
 */
#if defined(ONS_DEBUG_MODE) && ONS_DEBUG_MODE > 0
    #define SUNDRY_ASSERT(exp) ((exp)?0:(sundry_ferr("Assertation failed in %s at %u.\n", __FILE__, __LINE__),0))
    #define SUNDRY_FASSERT(exp, msg) ((exp)?0:(sundry_ferr("Assertation failed in %s at %u: %s\n", __FILE__, __LINE__, msg),0))
#else
    #define SUNDRY_ASSERT(exp)
    #define SUNDRY_FASSERT(exp, msg)
#endif
static void SUNDRY_ABORT(const char *format, ...) {
    va_list list;
    va_start(list, format);
    sundry_fdebug("ONS failed in %s at %u: ", __FILE__, __LINE__);
    sundry_fdebug(format, list);
    sundry_ferr("\n");
    va_end(list);
}
static void SUNDRY_DEBUG(const char *format, ...) {
#if ONS_DEBUG_MODE > 2
    va_list list;
    va_start(list, format);
    sundry_fdebug("Debug failed in %s at %u: ", __FILE__, __LINE__);
    sundry_vdebug(format, list);
    sundry_fdebug("\n");
    va_end(list);
#endif
}


#ifdef __cplusplus
}
#endif
#endif /* SUNDRY_INCLUDED_sundry_sundry_h */

