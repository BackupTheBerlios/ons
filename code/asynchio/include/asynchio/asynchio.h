/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 30. May 2009
 */

/* Main and public header of asynchio.
 * This header makes all IO functionality with all dependencies
 * visible to the user.
 */


#include <sundry/sundry.h>

#ifndef ASYNCHIO_INCLUDED_asynchio_asynchio_h
#define ASYNCHIO_INCLUDED_asynchio_asynchio_h
SUNDRY_EXTERN_C_BEGIN


/* Return codes.
 * This is a list of all codes which can be returned by functions of
 * asynchio. Every function returns either such a code or nothing.
 * Every return value is of type "unsigned int" and greater or equal to zero.
 * A function only returns a subset of these codes, please see the description
 * of each function for a list of valid return codes. A function always ONLY ADDS
 * information to the return code description. That is, only the combination of
 * the description here and in the function's comment will give you full information.
 *
 * A return code is always a key to a set of information. Every code is connected
 * to a \asyn_code_t structure which has the following information:
 * - \member: A bitset containing the groups which this code is member of. See below
 *            for valid groups.
 * - \str: A string (English language) which represents the code in text form.
 *         Please, this string is only for debug purposes! A return code can have
 *         different meanings depending on the function returning it. You should
 *         create own error strings with translations if you want to use it in you
 *         application. This string is only a generic description of the code.
 * - \name: This is the name of the error as a string. That is, the constant
 *          ASYN_DONE would be presented as "ASYN_DONE" here.
 * All strings are zero terminated in this structure.
 *
 * A return code can be member of special groups. A groups makes a set of codes more
 * general and thus easier to handle. You can simply check a return code for a group
 * and then perform the action which you connected to the group. Thus, you need much
 * less registered actions because there are much less groups than return codes.
 * The following groups are valid:
 * - ASYN_G_SUCCESS: All return codes in this group signal that the function succeeded.
 *
 * You can access the information of a return code with:
 * - ASYN_CODE(code): Returns a pointer to the \asyn_code_t structure related to a
 *                    return code.
 * - ASYN_ISIN(code, group): Returns 1 when return code \code is in group \group,
 *                           otherwise it returns 0.
 * Please be sure to only pass valid return codes to these functions, otherwise they
 * will segfault your application.
 *
 * The following return codes are defined:
 * - ASYN_DONE: This is guaranteed to be 0. It signals that the function succeeded successfully.
 * - ASYN_FAILED: This signals that the function failed. It does not provide any information
 *                why the function failed, so see the description of the function for related
 *                information. This is mostly returned when there is only a one possible failure.
 * - ASYN_BLOCKED: This signals that the function was not executed because it would block. That
 *                 is, the syscall behind would block because it either has not data or cannot
 *                 process further data. This is a temporary error. See the function description
 *                 what exactly blocked the call and how to avoid this error.
 * - ASYN_NONE: This marks an empty dataset. That is, the function had no data to return. Please
 *              see the function description to see what data is missing.
 * - ASYN_NOTINIT: The library was not initialized yet. Please do this before calling any function
 *                 of the library.
 * - ASYN_SYSCALL: A syscall error occurred. Not recoverable.
 * - ASYN_NOTSUPP: The kernel does not support the operation. This error will always occur when
 *                 this function is called with the same input. This is fatal to the user and the
 *                 application should drop network support when this occurs if it does not have
 *                 a workaround for this.
 * - ASYN_TOOMANY: Too many open file descriptors. Kernel rejected to create
 *                 further fds.
 * - ASYN_MEMFAIL: The kernel could not acquire enough resources to perform the action. This is
 *                 in no way related to the system's memory limit but to the sockets internal
 *                 buffer limit.
 * - ASYN_DENIED: Kernel denied access to perform this action.
 * - ASYN_LAST: This is *NO* valid return code! Don't use it as an return code! This simply is
 *              defined to the number of available error codes.
 */
enum asyn_ret_t {
    ASYN_DONE = 0,
    ASYN_FAILED,
    ASYN_BLOCKED,
    ASYN_NONE,
    ASYN_NOTINIT,
    ASYN_SYSCALL,
    ASYN_NOTSUPP,
    ASYN_TOOMANY,
    ASYN_MEMFAIL,
    ASYN_DENIED,
    ASYN_LAST
};
#define ASYN_G_SUCCESS 0x0001
typedef struct asyn_code_t {
    unsigned int member;
    const char *str;
    const char *name;
} asyn_code_t;
extern asyn_code_t asyn_codelist[ASYN_LAST];
#define ASYN_CODE(code) (&asyn_codelist[(code)])
#define ASYN_ISON(code, group) (ASYN_CODE(code)->member & (group))


/* Translation of IP4 and IP6 addresses.
 * asyn_str2addr translates a string representation into the
 * binary representation and returns ASYN_DONE on success.
 * Otherwise it returns ASYN_FAILED which means that the input
 * string could not be intepreted as a valid IP address.
 * The input string is the normal dotted IP format, either IPv4
 * or IPv6. In IPv4 the input can be formatted as plain decimal
 * format but also octal and hexadecimal.
 * The Ipv6 input is expected to conform to the standard.
 * asyn_addr2str does the opposite and always succeeds. The IPv6
 * address is written according to the standard, the IPv4 address,
 * due to a missing standard, is always formatted in the decimal
 * dotted format.
 *
 * asyn_str2addr:
 *  - \type: If the input is an IPv4 address, it is 0. Everything
 *           else is interpreted as IPv6 address. You can also use
 *           the constants ASYN_IPV4 and ASYN_IPV6.
 *  - \str: A zero terminated string pointing to the input.
 *  - \addr: Points to a buffer. If \type is 0, then the buffer must
 *           be 4 (ASYN_V4SIZE) bytes wide, otherwise it must be
 *           16 (ASYN_V6SIZE) bytes wide.
 *  - Returns: ASYN_DONE: Transformation was successfull.
 *             ASYN_FAILED: The input string could not be parsed.
 *
 * asyn_addr2str:
 *  - \type: Same as in \asyn_str2addr.
 *  - \addr: Points to the input address. If \type is 0, then the buffer must
 *           be 4 (ASYN_V4SIZE) bytes wide, otherwise it must be
 *           16 (ASYN_V6SIZE) bytes wide.
 *  - \str: Points to the output buffer. The output buffer must be at least
 *          46 bytes wide. After the transformation, the buffer will be zero
 *          terminated. The constant ASYN_STRLEN is defined to the minimal
 *          length of the buffer (46).
 *  - Returns: void
 *
 * ASYN_BUFSIZE is the size of a buffer which is big enough to old every kind
 *              of address.
 */
#define ASYN_IPV4 0
#define ASYN_IPV6 1
#define ASYN_V4SIZE 4
#define ASYN_V6SIZE 16
#define ASYN_BUFSIZE MEM_MAX(ASYN_V4SIZE, ASYN_V6SIZE)
#define ASYN_STRLEN 46
extern unsigned int asyn_str2addr(unsigned int type, const char *str, void *addr);
extern void asyn_addr2str(unsigned int type, const void *addr, char *str);


/* UDP object
 * This is a socket using the UDP protocol. You have to allocate this object
 * on the stack or heap and then pass it to \asyn_udp_init to create a new
 * UDP socket on it. After that you can pass the socket to all other asyn_udp_*
 * functions to operate on it. When you don't need the socket anymore, then
 * close it with \asyn_udp_close. Please be sure to free the object yourself when
 * you allocated it on the heap.
 *
 * There are several tasks which can be performed on the socket. You have to
 * take into account that every action on the socket may close the socket and
 * therefore make the socket unuseable. If that happens you cannot continue
 * using the socket. You should neither call \asyn_udp_close, but you should
 * simply deallocate the udp object and create a new one if desired.
 *
 * There are several options which can be set on the socket:
 * - ASYN_UDP_NBLOCK: Set socket into nonblocking state. Every function which
 *                    can return ASYN_BLOCKED will return this when the syscall
 *                    behind would block. You have to call the function again
 *                    later. How you can anticipate whether the call will block
 *                    depends on the call and is described in every function.
 * - ASYN_UDP_CLOEXEC: On some systems file descriptors are by default kept open
 *                     on an exec() call. With a fork()+exec() combination most
 *                     programs call their subhelpers (often 3rd party). If the
 *                     file descriptors are inherited by the subprocess the sub-
 *                     process may misuse them or gain access to data which it is
 *                     not supposed to read. This option ensures that the opened
 *                     file descriptor is under no circumstances inherited by the
 *                     subprocesses. Please take into account that not setting this
 *                     option does not explicitely make subprocesses inherit the
 *                     file descriptors. Some system simply do not support this.
 *                     However, by setting this options you can go sure that the
 *                     FD is not inherited.
 *                     It is recommended to pass this option directly to the init
 *                     function to make the kernel setting this option immediately
 *                     on the new socket.
 *
 * The following actions can be performed on the UDP object:
 * - sending: You can send data over the UDP object to an arbitrary destination.
 *            A single UDP object is not limited to a single destination.
 * - recv: You can receive data over the UDP object from all remote peers sending
 *         to your socket's address.
 * - getsockname: You can read the address of your local socket. This is the same
 *                which is set in the init function, however, you might have let
 *                the kernel decide what address to use. This functions will tell
 *                you the fixed address of the socket.
 * - setsockopt: You can modify the options of the UDP object on-the-fly.
 * - getlasterror: You can lookup the last error that was raised by the UDP object.
 * - getfd: You can get the system's file descriptor to directly use/modify it.
 *
 * \asyn_udp_t: Contains the UDP object information. \fd is the system's file
 *              descriptor. \error is the last error which occurred on this UDP
 *              object. Those members can either be directly accessed or with:
 *              - \asyn_udp_error: Returns the last error of object \udp.
 *              - \asyn_udp_fd: Returns the fd of object \udp.
 *              \error will be ASYN_NONE when no error occurred on the object yet.
 *              \fd is guaranteed to be always a valid file descriptor (fd >= 0).
 *
 * \asyn_udp_init: Initializes a new UDP object. The space has to be allocated by
 *                 the user and is not freed by any function operating on this
 *                 object so the user has to free it themself.
 *      - \udp: Points to the allocated space where to initialize the UDP object.
 *      - \opts: Specifies UDP options which are set on the socket, if you don't want
 *               want to set an option, pass 0.
 *      - \type: Type of the address to bind the socket to.
 *      - \addr: Pointer of the address to bind the socket to. If NULL then it is bound
 *               to the default address.
 *      - \port: The port this socket should be bound to. If 0 then is it bound to a
 *               random port.
 *      - Returns: ASYN_DONE: The socket was created successfully with all options set
 *                            and bound to the given address.
 *
 * \asyn_udp_close: Closes the socket of the UDP object. The library does not allocate
 *                  any memory to this object so this function does not free anything
 *                  except the socket file descriptor.
 *      - \udp: Points to the UDP object.
 *      - Returns: void
 *
 * \asyn_udp_ctl: Modifies the options which are set on the socket. 
 */
typedef struct asyn_udp_t {
    signed int fd;
    unsigned int error;
} asyn_udp_t;
#define asyn_udp_error(udp) ((udp)->error)
#define asyn_udp_fd(udp) ((udp)->fd)
#define ASYN_UDP_NBLOCK 0x0001
#define ASYN_UDP_CLOEXEC 0x0002
extern unsigned int asyn_udp_init(asyn_udp_t *udp, unsigned int opts, unsigned int type, const void *addr, unsigned int port);
extern void asyn_udp_close(asyn_udp_t *udp);
extern unsigned int asyn_udp_ctl(asyn_udp_t *udp, unsigned int opts);
extern unsigned int asyn_udp_addr(asyn_udp_t *udp, unsigned int *type, void *addr);
extern unsigned int asyn_udp_send(asyn_udp_t *udp, const void *buf, size_t *size);
extern unsigned int asyn_udp_recv(asyn_udp_t *udp, void *buf, size_t *size);





#if 0
/* AsynchIO Design:
 * AsynchIO implements an IO object which can have any backend and therefore
 * can be used for any possible task. The main actions which can be done
 * on an IO object are reading and writing. All other actions must use the
 * "control" interface which may differ between the several backends.
 *
 * The main IO tasks are reading/writing synchronously AND asynchronously
 * and waiting for events. Asynchronous IO is achieved by setting the FD
 * to non-blocking state. Every backend currently supports something
 * similar to a non-blocking state, however, if there will be a backend
 * that does not support non-blocking states, then this is implemented by
 * putting a pipe between the backend and frontend and threading the backend.
 *
 * Furthermore, every IO object can be accessed with a system's file descriptor.
 * This allows to use the common select() system call or faster equivalents like
 * epoll/kqueue/etc. and popular event engines with all IO objects. On some
 * systems there are IO objects which are not directly available as file
 * descriptors. Then we place a pipe between the backend and frontend and the
 * user can poll on the pipe.
 */

/* IO Object:
 * This part implements the main filedescriptor and a low level API to create
 * new objects, write and read from them and finally close them.
 * It is highly technical and the user might use more high-level APIs. This
 * object uses several backends which implement the different objects on
 * various operating systems.
 */

/* IO System:
 * The IO System implements several fast and easy ways to send and read from
 * an IO Object and functions which ease the use of the "control" interface.
 * This API is the base for every protocol implementation.
 */


#include <stdint.h>


/* IO objects. */
typedef struct asyn_obj_t {
    unsigned int type;
    unsigned int opts;
    unsigned int error;
    signed int fd;
    void *io;
} asyn_obj_t;

/* Types of IO objects. */
enum asyn_type_t {
    ASYN_UDP4,
    ASYN_LAST
};

/* Generic options available on all types. */
typedef uint16_t asyn_opt_t;
#define ASYN_NONE       0x0000
#define ASYN_SET        0x0001  /* Set the options. */
#define ASYN_UNSET      0x0002  /* Unset the options. */
#define ASYN_NBLOCK     0x0004  /* Makes the functions return ASYN_BLOCKED when the call would block. */

/* Basic return values. */
enum asyn_ret_t {
    ASYN_BLOCKED = -2,      /* The call would block and should be recalled later. */
    ASYN_FAILED = -1,      /* The call failed but the object is still usable. */
    ASYN_CLOSED = 0,        /* The object was closed due to a failure in the call. */
    ASYN_SUCCESS = 1        /* The call succeeded. */
};

/* Error codes. */
enum asyn_error_t {
    ASYN_E_SUCCESS = 0,
    ASYN_E_INVALTYPE,       /* Invalid asyn_type_t type. */
    ASYN_E_NOTSUPP,         /* Operation not supported. */
    ASYN_E_DENIED,          /* The underlying syscall denied access. */
    ASYN_E_FDPFULL,         /* Process' FD table is full. */
    ASYN_E_FDSFULL,         /* System's FD table is full. */
    ASYN_E_MEMFAIL,         /* Kernel could not claim enough memory for this operation. */
    ASYN_E_SYSCALL,         /* Syscall returned unknown error code. This immediately closes the object. */
    ASYN_E_INTR,            /* Syscall was interrupted by signal. This is catched by the library by default and is never returned. */
    ASYN_E_BADFD,           /* Bad file descriptor. */
    ASYN_E_NOTCONN,         /* Object is not connected. */
    ASYN_E_ISCONN,          /* Object is connected. */
    ASYN_E_ADDRINUSE,       /* Address already in use. */
    ASYN_E_ADDRNOTAVAIL,    /* Address not available. */
    ASYN_E_IO,              /* IO error. */
    ASYN_E_NOTDIR,          /* No valid directory. */
    ASYN_E_ISDIR,           /* Is a directory. */
    ASYN_E_NOSPACELEFT,     /* No space left on device. */
    ASYN_E_ROFS,            /* ReadOnly filesystem. */
    ASYN_E_LOOP,            /* Wrong/Toomany symbolic links. */
    ASYN_E_FNAMETOOLONG,    /* File name too long. */
    ASYN_E_QUOTA,           /* Quota exceeded. */
    ASYN_E_HOSTDOWN,        /* Remote host is down. */
    ASYN_E_HOSTUNREACH,     /* Remote host is unreachable. */
    ASYN_E_NETDOWN,         /* Network is down. */
    ASYN_E_NETUNREACH,      /* Network is unreachable. */
    ASYN_E_TIMEDOUT,        /* Operation timed out. */
    ASYN_E_REFUSED,         /* Remote host refused operation. */
    ASYN_E_ALREADY,         /* Same operation currently in progress or already done. */
    ASYN_E_NONBLOCK,        /* Operation continues in background. */
    ASYN_E_NOFILE,          /* No such file or directory. */
    ASYN_E_ABORTED,         /* Connection was aborted. */
    ASYN_E_MSGSIZE,         /* Message size is too big. */
    ASYN_E_PIPE,            /* Local end has already been shutdown. */
    ASYN_E_RESET,           /* Remote end has reset the connection. */
    ASYN_E_NETRESET,        /* Connection dropped due to network reset. */
    ASYN_E_SHUTDOWN,        /* Local end is already shutdown. */
    ASYN_E_TOOBIG,          /* File is getting too large. */
    ASYN_E_NOINIT,          /* The library was not initialized, yet. */
    ASYN_E_LAST
};

/* (De)Initializes the library. */
extern unsigned int asyn_init();
extern void asyn_deinit();

/* Creates/Frees IO objects. */
extern unsigned int asyn_open(asyn_obj_t **obj, unsigned int type, unsigned int opts, ...);
extern unsigned int asyn_merge(asyn_obj_t **obj, unsigned int type, unsigned int opts, ...);
extern void asyn_close(asyn_obj_t *obj);
extern unsigned int asyn_err(asyn_obj_t *obj);
extern signed int asyn_fd(asyn_obj_t *obj);

/* Modifies a file descriptor. */
extern signed int asyn_ctrl(asyn_obj_t *obj, asyn_opt_t opt, ...);

/* Basic IO. */
extern signed int asyn_write(asyn_obj_t *obj, const void *buf, size_t *len, ...);
extern signed int asyn_read(asyn_obj_t *obj, void *buf, size_t *len, ...);


/* UDP4 backend. */
#define ASYN_UDP4_CLOEXEC 0x00010000        /* FD is closed on exec(). */
#endif

SUNDRY_EXTERN_C_END
#endif /* ASYNCHIO_INCLUDED_asynchio_asynchio_h */

