/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 27. May 2009
 */

/* Main and public header of asynchio.
 * This header makes all IO functionality with all dependencies
 * visible to the user.
 */


#include <sundry/sundry.h>

#ifndef ASYNCHIO_INCLUDED_asynchio_asynchio_h
#define ASYNCHIO_INCLUDED_asynchio_asynchio_h
SUNDRY_EXTERN_C_BEGIN


/* AsynchIO Design:
 * AsynchIO implements an IO object similar to the Berkeley Sockets which
 * can have different backends. However, you can access the object with
 * a simple pointer and write/read from it like from Berkeley Sockets.
 *
 * Since some system do not provide such integral file descriptors for
 * some tasks, the backend may use complex system APIs. However, the user
 * does not have to bother with that but this allows to implement nearly
 * every IO system as a backend here.
 *
 * The main IO tasks are reading/writing synchronously AND asynchronously
 * and waiting for events. Asynchronous IO is achieved by setting the FD
 * to non-blocking state. Every backend currently supports something
 * similar to a non-blocking state, however, if there will be a backend
 * that does not support non-blocking states, then this might be done with
 * a threading architecture.
 *
 * The event engine is another big part of AsynchIO. It is probably the
 * most complex part, however, it can be used like common other event
 * engines but also allows APIs which are not common in event eninges but
 * were cloned from APIs like POSIX AIO.
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
 * an IO Object.
 * This API is the base for every protocol implementation.
 */

/* Event Engine:
 * The event eninge allows to poll for events on objects. It is possible to
 * add ANY kind of object to this event engine, however, if the operating
 * system does not support polling on this kind of object, the polling mechanism
 * is implemented with signals, pipes or threading mechanisms which may slow
 * down the event eninge.
 * Please read the documentation to get information on performance issues and
 * how to avoid slow API calls.
 */


/* IO objects. */
typedef struct asyn_obj_t {
    unsigned int type;
    unsigned int opts;
    unsigned int error;
    void *io;
} asyn_obj_t;

/* Types of IO objects. */
enum asyn_type_t {
    ASYN_UDP,
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
    ASYN_FAILURE = -1,      /* The call failed but the object is still usable. */
    ASYN_CLOSED = 0,        /* The object was closed due to a failure in the call. */
    ASYN_SUCCESS = 1        /* The call succeeded. */
};

/* Error codes. */
enum asyn_error_t {
    ASYN_E_SUCCESS = 0,
    ASYN_E_INVALTYPE,       /* Invalid asyn_type_t type. */
    ASYN_E_NOTSUPP,         /* Operation not supported. */
    ASYN_E_LAST
};

/* (De)Initializes the library. */
extern unsigned int asyn_init();
extern void asyn_deinit();

/* Creates/Frees IO objects. */
extern unsigned int asyn_open(asyn_obj_t **obj, unsigned int type, unsigned int opts);
extern unsigned int asyn_merge(asyn_obj_t **obj, unsigned int type, unsigned int opts, ...);
extern void asyn_close(asyn_obj_t *obj);
extern unsigned int asyn_err(asyn_obj_t *obj);

/* Modifies a file descriptor. */
extern signed int asyn_ctrl(asyn_obj_t *obj, asyn_opt_t opt, ...);

/* Basic IO. */
extern signed int asyn_write(asyn_obj_t *obj, const void *buf, size_t *len);
extern signed int asyn_read(asyn_obj_t *obj, void *buf, size_t *len);


SUNDRY_EXTERN_C_END
#endif /* ASYNCHIO_INCLUDED_asynchio_asynchio_h */

