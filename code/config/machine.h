/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/* ONS configuration
 * You need to modify this file before compiling ONS. The defaults in
 * this file are not apropriate for most platforms.
 *
 * NOTES ON CROSS-COMPILING:
 *  If you're cross-compiling then please adjust the settings to the
 *  target-platform instead of the build-platform.
 *
 * What to do?
 * Please read each comment block in this file thoroughly. The first block
 * asks you whether you want to use one of the shipped configurations. If
 * one of these configuration-files fits to your platform, then you can skip
 * all further steps in this file.
 * If none of these files fits, you need to read the rest and create an own
 * configuration.
 */

/* Default configurations
 * Please uncomment the line which fits to your system. If your system only
 * slightly differs from a config mentioned here, you can simply edit the file
 * which is included with this directive.
 *
 * #include "macosx.machine.h"
 *    Uncomment this if you use MacOS-X as target platform. All versions are
 *    currently supported. Be sure that your building environment is the one
 *    shipped with the XCode program by Apple. That is, the compiler is a
 *    modified version of GCC and the standard-C-library is from apple.
 *    The architecture is either intel-based or powerPC based or 68k based.
 *
 * #include "generic.machine.h"
 *    This expects a generic UNIX system with traditional paths and libraries.
 *    The architecture is expected to be x86 compatible.
 *    This file is deprecated and should only be used in exotic environments if
 *    you have no idea what your system is.
 */
#include "macosx.machine.h"
/* #include "generic.machine.h" */


/* If none of the configuration files above fit to your needs, then please
 * continue with the next configuration comment below. However, if you uncommented
 * a line above then you should stop here unless you want to manually change some
 * settings here. It is always recommended to use a shipped configuration. If such
 * a configuration is not available then you can also contact the official developers
 * or the maintainers of the packet of your distribution which then will create a
 * configuration for your platform.
 */


/* Manual configuration
 * Below you can manually configure ONS. The configuration is splitted into groups
 * where each group can define several constants. You can either choose between
 * several constants and you have to define one of them. Then the options are exclusive
 * and it wouldn't make sense to define more of them.
 * However, some other options are simple on/off options which define whether a special
 * functionality is available on your platform.
 *
 * For example there could be a questions what compiler you are using and it could list
 * several constants:
 *   ONS_CC_GCC
 *   ONS_CC_ICC
 *   ...
 *   ONS_CC_LLVM
 * And then you obviously had to define one of them. It would not be sufficient to define
 * none or to define more than one.
 * However, there could also be a question
 * asking whether your compiler supports C99:
 *   ONS_CC_SUPPORTS_C99
 * Then you apparently had to either define it or not.
 * The first type is called [SELECT] the second type is called [OPTION].
 */


/* CONFIG: Architecture
 * SHORT: ARCH
 *
 * This part of the config defines the architecture that you are using. You can do
 * this by defining one of the following constants [SELECT]:
 *   ONS_ARCH_8BIT: By defining this you declare that the target architecture treats
 *                  one byte equally to 8 bits. This is true on 99.999% of all
 *                  architectures on this world. I have never seen an architecture
 *                  which does not fit into this group, however, I've heard of one
 *                  that works in mobile devices.
 */
/* #define ONS_ARCH_8BIT */


/* CONFIG: Compiler
 * SHORT: CC
 *
 * The following constant MUST be defined under all circumstances. It defines that you
 * have a compiler that supports:
 *    - Full ISO-C89 compatible.
 *    - Variable argument lists of macros with __VA_ARGS__.
 *    - "inline" keyword
 *    - "long long int" must be supported.
 *    - C99 subobject initialization.
 *    - The binary representation of a null-pointer must be zero in any type.
 *    - Integer types must not have padding.
 *    - The C compiler must halt (and return != 0) on compilation errors.
 *    - The C compiler must support indented macros. For instance: "    #define ANYTHING"
 *    - The header "stdint.h" must be available as described in C99.
 *    - The C compiler must support anonymous unions/structs/enums.
 *   ONS_CC_STD: We could omit this constant, but simply define it and we can go sure that
 *               you have read the requirements for the C compiler. Compilers like GCC, LLVM,
 *               icc and others support this.
 */
/* #define ONS_CC_STD */


/* CONFIG: Thread
 * SHORT: THREAD
 *
 * This is defined if a threading library is available. The threading library
 * has to support: starting threads, waiting for threads to finish and an interface
 * for mutexes.
 * Exyctly *ONE* of the following backends *MUST* be defined [SELECT].
 *
 *   ONS_THREAD_PTHREAD:
 *          The thread backend is the POSIX threads library "pthread".
 *          The header <pthread.h> must be in your system's include paths.
 *          The library "libpthread.so" must be in your libpath.
 *
 *   ONS_THREAD_WIN:
 *          The thread backend is the Windows API.
 *
 * When ONS_THREAD_PTHREAD is enabled the following option can be set:
 *   ONS_THREAD_PTHREAD_TMR: pthread supports the TMR (timer) extension. That is
 *                           pthread_mutex_timedlock is available.
 */
/* #define ONS_THREAD_PTHREAD */
/* #define ONS_THREAD_WIN */
/* #define ONS_THREAD_PTHREAD_TMR */


/* CONFIG: Time
 * SHORT: TIME
 *
 * This defines whether time-controlling functions are available on the platform.
 * Exactly *ONE* of the following backends must be specified [SELECT].
 *
 *   ONS_TIME_GTOD:
 *          The "gettimeofday()" function is available on your machine through <sys/time.h>
 *          and conforms to POSIX.
 *
 *   ONS_TIME_WIN [void]
 *          The Windows API function GetSystemTimeAsFileTime() is available.
 */
/* #define ONS_TIME_GTOD */
/* #define ONS_TIME_WIN */


/* CONFIG: Debug-facility
 * SHORT: DEBUG
 *
 * This defines whether debug messages should be included in the library.
 *
 *   ONS_DEBUG_MODE [0..4]
 *          This is defined if debug behaviour shall be included. Each library
 *          of ONS has to decide whether it includes debug facilities.
 *          However, if this macro is not defined, debug facilities must not
 *          be included.
 *          If the macro is defined, it expands to an integer describing the
 *          debug level. A higher level always includes more debug output.
 *          An high level includes all lower levels.
 *      Level:
 *      - 0: Equals to not defining this macro. No debug information is included.
 *      - 1: Basic security mechanism are included like assertations.
 *      - 2: Debug messages are printed to stderr on fatal errors.
 *      - 3: Debug messages are printed to stderr on warnings and unexpected function calls.
 *           Reporting them to the developers will help improving ONS.
 *      - 4: Currently not used.
 */
/* #define ONS_DEBUG_MODE 1 */

