/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 22. December 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 15. March 2009
 */

/* Main and public header of ONS.
 *
 * Every operating system has a different API to access the machine.
 * However, some parts of the API are available on more than one system.
 * This library does not wrap all these APIs, nor does it replace them.
 * But this library defines what API calls are available on the system
 * and, hence, libraries depending on this one can check which API they
 * have to use.
 *
 * This library, obviously, does not support every available API, but
 * supports only the APIs which are used by other libraries depending
 * on ONS.
 * Each supported API is described below together with all macros
 * describing this API.
 *
 * The description of the available API is done with macros. Each part
 * of an API of the system has one or more macros which represent this
 * part of the API. There is always one macro which is set or not set
 * depending on whether this part of the system's API is available.
 * For instance if your system has the gettimeofday() function call the
 * macro ONS_GETTIMEOFDAY is defined. If this function is not available,
 * the macro is not defined.
 * The value, which this macro is defined to, is not specified. Most of these
 * macros expand to (void).
 * Additionally, there may be macros which mark some variations of the
 * API. For instance the gettimeofday() call does not correctly validate
 * argument 2 on this machine (only a fictive example), then the macro
 * ONS_GETTIMEOFDAY_NOARG2 is defined.
 * Some macros are even defined to some values/strings which can be used
 * on the source.
 *
 * This file checks which system is used at compile time and sets the right
 * macros which then define the available API.
 * Additionally, some *REALLY* general declarations/functions are included
 * in this file. These are kept as less as possible.
 *
 * Common systems, like Linux, have a single header which describes
 * these systems. If ONS gets bigger there will probably be more single
 * headers for each system.
 * However, all other systems are served by a generic header which tries to
 * set the most common values.
 *
 * David Herrmann -- 2009/03/15
 */


#ifndef ONS_INCLUDED_ons_ons_h
#define ONS_INCLUDED_ons_ons_h
#ifdef __cplusplus
extern 'C' {
#endif


/* API Description
 * Following, each supported API is described. The description is always
 * built up the same way. The first line gives a name for the API which
 * is used in documentation and other text documents.
 * The second line defines the macro which is defined when this API is
 * available. After this, a short description of the API is provided.
 * This description described the API and the expected default behaviour.
 * If there are known systems which differ from this default behaviour,
 * the API may define several other macros which define whether and how
 * the behaviour differs on this machine.
 *
 * If no value is specified behind a macro, the macro is not define to
 * a value. However, if a value is defined, the following special values
 * can be assigned:
 * [void] This macro expands to no defined value.
 * [-5..10] A range of numbers this macro is defined to.
 * ["string"] A special string this macro is defined to.
 * [special] A value which must be described in the comment of the macro.
 */


/* API: Machine
 * macro: ONS_MACHINE [void]
 *
 * This API is a meta API and is always defined. This API describes the
 * physical machine where this header is installed on. If the library is
 * copied onto another physical machine these values may change. However,
 * most binaries won't work on more than one architecture.
 *
 * macro: ONS_MACHINE_BIGENDIAN [void]
 * macro: ONS_MACHINE_LITTLEENDIAN [void]
 * purport: Exactly one of both macros must be defined. It specifies whether
 *          the machine is a BigEndian or LittleEndian machine.
 *
 * macro: ONS_MACHINE_PACK [void]
 * macro: ONS_MACHINE_ATTR [void]
 * purport: At least one of both must be defined. The first specifies that
 *          the system supports the '#pragma pack()' directive as supported
 *          by the GCC. The second macro specifies whether the
 *          __attribute__((__packed__)) attribute is supported as implemented
 *          by the GCC.
 *
 * macro: ONS_MACHINE_IPV6 [void]
 * purport: If this macro is defined, the library shall support IPv6. This
 *          macro does not imply any specific behaviour, however, if this macro
 *          is not defined, IPv6 support must not be included in the library.
 *
 * macro: ONS_MACHINE_DEBUG [1..5]
 * purport: This is defined if debug behaviour shall be included. Each library
 *          depending on ONS has to decide whether it includes debug facilities.
 *          However, if this macro is not defined, debug facilities must not
 *          be included.
 *          If the macro is defined, it expands to an integer describing the
 *          debug level. A higher level always includes more debug output.
 */


/* API: System
 * macro: ONS_SYSTEM [void]
 *
 * This API is a meta API. It must be available on every system. It's purpose
 * is to describe the behaviour which is required from every system. A system
 * which does not conform to this API will not be compatible to ONS.
 * Following a list of all behaviours/functions/headers/etc. which are required
 * to define this macro:
 *  - The ISO-C89 standard must be supported by the compiler and the standard
 *    library must be available.
 *  - The __VA_ARGS__ macro must be supported as described in the ISO-C99 standard.
 *  - The "inline" keyword must be supported as described in the ISO-C99 standard.
 *  - The binary representation of a null-pointer must be zero in any type.
 *  - Integer types must not have padding.
 *  - The C compiler must halt (and return != 0) on compilation errors.
 *  - The C compiler must support indented macros. For instance: "    #define ANYTHING"
 *  - One byte must be equal to 8 bit.
 *  - This header must make fixed-size integers available as described in stdint.h
 *    in ISO-C99.
 *  - The C compiler must support anonymous unions/structs/enums.
 */


/* API: String
 * macro: ONS_STRING [void]
 *
 * This API is always defined. It describes whether the string API as described
 * in ISO-C89 is available.
 *
 * macro: ONS_STRING_STRNLEN [void]
 * purport: strnlen() is not part of ISO-C89 and therefore is not available on
 *          all systems. However, if it is available on the system, this macro
 *          has to be defined and the required headers have to be included.
 */


/* The "machine.h" file is created on compile time. We include this
 * created header here. This header includes several ONS_CONF_* macros which
 * are set by the configuration script on compile time and are used to set the
 * right API macros. These ONS_CONF_* macros shall not be used directly by
 * any application.
 */
#include <ons/machine.h>


/* Now we have to define the right macros.
 * If we are a 32bit or 64bit windows machine, we include a predefined header
 * for this system.
 */
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS_)
    #include <ons/windows.h>
#elif defined(__linux) || defined(__linux__) || defined(linux)
    #include <ons/linux.h>
#else
    #include <ons/generic.h>
#endif


/* Some of the macros of the APIs have long names. We declare short forms of some macros here. */
#ifdef ONS_MACHINE_DEBUG
    #define ONS_DEBUG ONS_MACHINE_DEBUG
#endif


/* After describing the available API, this header also defines some macros/variables/types/etc.
 * which are *REALLY* general and probably be used in all libraries depending on it.
 */


/* NULL must be defined in every header we use, thus, include stddef.h. */
#include <stddef.h>


/* To make all headers available in C++ they need to define "extern 'C'".
 * This macros defines "extern 'C'" if the compiler is a C++ compiler.
 * At the end of the header you must close "extern 'C'".
 *
 * ONS_EXTERN_C: Defined as "extern 'C'" if __cplusplus is defined.
 * ONS_EXTERN_C_BEGIN: Defined as "extern 'C' {" if __cplusplus is defined.
 * ONS_EXTERN_C_END: Defined as "}" if __cplusplus is defined.
 */
#ifdef __cplusplus
    #define ONS_EXTERN_C extern 'C'
    #define ONS_EXTERN_C_BEGIN ONS_EXTERN_C {
    #define ONS_EXTERN_C_END }
#else
    #define ONS_EXTERN_C
    #define ONS_EXTERN_C_BEGIN
    #define ONS_EXTERN_C_END
#endif


/* Include the required headers. */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


/* This function prints an error message and exits the application.
 * If \format is NULL, a default message is printed.
 */
static inline void ons_verr(const char *format, va_list list) {
    if(format != NULL) vfprintf(stderr, format, list);
    else fprintf(stderr, "ONS discovered a fatal error. No error message specified; Exiting...\n");
    abort();
}
static inline void ons_ferr(const char *format, ...) {
    va_list list;

    va_start(list, format);
    ons_verr(format, list);
    va_end(list);

    abort();
}


/* This function prints an error message and returns.
 * If \format is NULL, a default message is printed.
 */
static inline void ons_vdebug(const char *format, va_list list) {
    if(format != NULL) vfprintf(stderr, format, list);
    else fprintf(stderr, "ONS discovered a debug error. No debug message specified; Ignoring...\n");
}
static inline void ons_fdebug(const char *format, ...) {
    va_list list;

    va_start(list, format);
    ons_vdebug(format, list);
    va_end(list);
}


/* ONS_ASSERT: This macro is equal to ISO-c89 assert() but does not depend on NDEBUG but on ONS_DEBUG.
 * ONS_DMSG: (debug message) This macro prints a message to stdout that a bug occurred and should be
 *                           reported when ONS_DEBUG is defined.
 * ONS_ABORT: Prints a message and aborts.
 */
#ifdef ONS_DEBUG
    #define ONS_ASSERT(x) ((x)?(ons_ferr("ONS assertation failed in %s at %u: %s\n", __FILE__, __LINE__, msg),0):0)
    #define ONS_DMSG(...) (ons_fdebug("ONS debug failed in %s at %u: ", __FILE__, __LINE__), ons_fdebug(__VA_ARGS__), ons_fdebug("\n"), 0)
#else
    #define ONS_ASSERT(x)
    #define ONS_DMSG(...)
#endif
#define ONS_ABORT(...) (ons_fdebug("ONS failed in %s at %u: ", __FILE__, __LINE__), ons_fdebug(__VA_ARGS__), ons_ferr("\n"), 0)


/* Structure packing mechanism.
 * If __attribute__((__packed__)) is supported, we declare ONS_ATTR_PACK
 * to __attribute__((__packed__)).
 */
#ifdef ONS_MACHINE_ATTR
    #define ONS_ATTR_PACK __attribute__((__packed__))
#else
    #define ONS_ATTR_PACK
#endif


#ifdef __cplusplus
}
#endif
#endif /* ONS_INCLUDED_ons_ons_h */

