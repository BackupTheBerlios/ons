/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 13. May 2009
 */

/* Main and public header of asynchio.
 * This header makes all IO functionality with all dependencies
 * visible to the user.
 */

#include <ons/ons.h>

#ifndef ONS_INCLUDED_asynchio_asynchio_h
#define ONS_INCLUDED_asynchio_asynchio_h
ONS_EXTERN_C_BEGIN


/* AsynchIO Design:
 * AsynchIO implements an IO object similar to the Berkeley Sockets which
 * can have different backends. However, you can access the object with
 * a simple integer and write/read from it like from Berkeley Sockets.
 *
 * Since some system do not provide such integral file descriptors for
 * some tasks, the backend may use complex system APIs. However, the user
 * does not have to bother with that but this allows to implement nearly
 * every IO system as a backend here.
 *
 * The main IO tasks are reading/writing synchronously AND asynchronously
 * and waiting for events. Asynchronous IO is achieved by setting the FD
 * to non-blocking state. If a backend does not support non-blocking FDs,
 * a threading architecture is used to simulate it.
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
 * an IO Object. It should be used for accessing the IO Object instead of
 * the low-level API because it uses special system calls which may speed up
 * the functions which cannot be used in the low-level API.
 * This API is the base for every protocol implementation. It also provides
 * buffered reads/writes and really basic parser mechanisms like reading a
 * whole line.
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
typedef size_t io_fd_t;
#define IO_INVALID_FD 0

/* Types of IO objects. */
enum {
    IO_UDP,
    IO_LAST
};

/* Creates/Frees IO objects. */
extern io_fd_t io_open(ons_err_t *err, signed int type);
extern io_fd_t io_merge(ons_err_t *err, signed int type);
extern void io_close(io_fd_t fd);

/* Modifies a file descriptor. */
extern unsigned int io_control(io_fd_t fd, ...);
extern ons_err_t io_err(io_fd_t fd);

/* Basic IO. */
extern size_t io_write(io_fd_t fd, const void *buf, size_t len);
extern size_t io_read(io_fd_t fd, void *buf, size_t len);


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_asynchio_asynchio_h */

