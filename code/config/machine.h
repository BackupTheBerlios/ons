/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 25. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 17. June 2009
 */

/* ONS configuration
 * ONS can be configured with a set of constants. This file described all those
 * constants, however, this file does not define them, but it includes
 * a shipped configuration file for a specific system which defines these
 * constants. This file, though, does not check your system but needs an external
 * program that tells him which file to include.
 *
 * This file includes the following configurations when the right constants are
 * defined:
 *  - ONS_SYS_MACOS: "macos.machine.h"
 *  - ONS_SYS_LINUX: "linux.machine.h"
 *  - ONS_SYS_WINDOWS: "windows.machine.h"
 *  - if none of them is defined it includes "generic.machine.h"
 * Please look into the configuration files to get information on which systems
 * are supported by them.
 *
 * If your system is not supported by one of these configurations, you need to
 * modify the generic one. Simply read all comments below and in generic.machine.h and
 * check whether your system supports the functions described. If so, then define
 * the constant in the file "generic.machine.h".
 * If you've done this your configuration consists of a bunch of constants which
 * describe your system which is now ready to use for ONS. Reading the other
 * shipped configurations might also help to create your own one.
 */

/* This includes the right configuration. If no configuration is defined, this
 * includes "generic.machine.h" which has to be modified to fit to your platform.
 */
#if defined(ONS_SYS_MACOS)
    #include "macos.machine.h"
#elif defined(ONS_SYS_LINUX)
    #include "linux.machine.h"
#elif defined(ONS_SYS_WINDOWS)
    #include "windows.machine.h"
#else
    #define ONS_SYS_GENERIC
    #include "generic.machine.h"
#endif


/* Configuration constants
 *
 * The comments below describe all available constants which can be used to configure
 * ONS. It is important that you know your platform very good to create your own
 * configuration.
 * If you have no idea what to do here, please contact us and we will create a
 * configuration for your platform which then will be autodetected by the build scripts.
 */


/* Required functions
 *
 * ONS requires your system to support various functions. These functions are essential
 * and must be available on your platform. There is *NO* constant which says that these
 * functions are available because ONS will never compile if they are not available.
 *
 * The functions are:
 *  - ONS must be compiled with an ISO-C89 compatible C compiler. It must support all
 *    aspects of ISO-C89. Compilers like GCC, ICC, LLVM and MS-VSC++ are known to work
 *    well.
 *  - The binary representation of NULL pointers must be 0 in all pointer types. This
 *    is true on all platforms I know, however, ISO-C89 allows also other systems.
 *  - Integer types must not have padding. This is also true on every system I know
 *    but ISO-C89 again allows other systems.
 *  - stdint.h must be available on your platform.
 */


/* Architecture
 *
 * If your architecture is LittleEndian then define ONS_ARCH_LITTLEENDIAN. If it is
 * BigEndian then define ONS_ARCH_BIGENDIAN.
 * One of both HAS to be defined. Other endians like MIDDLEENDIAN are not supported
 * and I don't know any system using those.
 */
/* #define ONS_ARCH_LITTLEENDIAN */
/* #define ONS_ARCH_BIGENDIAN */


/* Threading facility
 *
 * If the "pthread" library is available on your system, then please define ONS_THREAD_PTHREAD.
 * If the Windows Threading Library is available through the Windows-API then define
 * ONS_THREAD_WIN.
 * At least one of them MUST be defined.
 *
 * If you defined ONS_THREAD_PTHREAD and the TMR (timer) extension is available on your
 * platform, then you can additionally define ONS_THREAD_PTHREAD_TMR which uses the timing
 * functions then.
 */
/* #define ONS_THREAD_WIN */
/* #define ONS_THREAD_PTHREAD */
/* #define ONS_THREAD_PTHREAD_TMR */


/* Precise time backend
 *
 * If the GetTimeOfDay() function is available on your platform through <sys/time.h> then define
 * ONS_TIME_GTOD. If the windows function GetSystemTimeAsFileTime() is available through the Windows
 * API then define ONS_TIME_WIN.
 * One of them must be defined. *_GTOD includes the usleep() systcall and *_WIN includes the sleep()
 * syscall.
 */
/* #define ONS_TIME_GTOD */
/* #define ONS_TIME_WIN */


/* Socket API
 *
 * The Berkeley Socket API must be (at least partly) supported by your system. Nearly
 * every system supports other BSD extensions so only the basic tasks are required
 * everything else can be configured with the macros below.
 *
 * Since the Berkeley Socket API is not expected to be fully supported on every system
 * we have workarounds for rarely used functions which are not available in every
 * Berkeley-API implementation:
 * - If socket() supports the binary options SOCK_NONBLOCK and SOCK_CLOEXEC then
 *   define ONS_SOCKET_EXTSOCK.
 * - If the typical berkeley socket headers like sys/socket.h are available, then
 *   define ONS_SOCKET_BERKELEY_HEADERS.
 * - If the API is available through WindowsAPI then define ONS_SOCKET_WIN_HEADERS.
 * - If the fcntl() syscall is available, define ONS_SOCKET_FCNTL.
 * - If the ioctl() syscall is available, define ONS_SOCKET_IOCTL.
 * - If the ioctlsocket() function is available, define ONS_SOCKET_IOCTLSOCKET.
 * - If the BSD address structures have a "len" member, then define ONS_SOCKET_ALEN.
 *
 * One of *_FCNTL, *_IOCTL, *_IOCTLSOCKET must be defined.
 * A combination of ONS_SOCKET_WIN_HEADERS with one of the following is invalid:
 * - ONS_SOCKET_EXTSOCK
 * - ONS_SOCKET_FCNTL
 * - ONS_SOCKET_IOCTL
 */
/* #define ONS_SOCKET_EXTSOCK */
/* #define ONS_SOCKET_BERKELEY_HEADERS */
/* #define ONS_SOCKET_WIN_HEADERS */
/* #define ONS_SOCKET_FCNTL */
/* #define ONS_SOCKET_IOCTL */
/* #define ONS_SOCKET_IOCTLSOCKET */
/* #define ONS_SOCKET_ALEN */


/* Debug mode
 *
 * This defines whether debug messages should be included in the library.
 *
 *   ONS_DEBUG_MODE
 *          This is defined if debug behaviour shall be included. Each library
 *          of ONS has to decide whether it includes debug facilities.
 *          However, if this macro is not defined, debug facilities must not
 *          be included.
 *
 * This macro is not defined by the backends but inside this file. If you downloaded a
 * release version this is probably not defined, if you downloaded a beta version this is probably
 * defined.
 */
#define ONS_DEBUG_MODE

