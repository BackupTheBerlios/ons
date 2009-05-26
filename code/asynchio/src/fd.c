/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 14. May 2009
 */

/* Generic File Descriptors
 * This API controls the file descriptors and describes the API
 * used by the backends. It delegates the work to the right
 * backend and checks for valid parameters.
 */


#include "asynchio/asynchio.h"
#include "memoria/memoria.h"
#include "sundry/thread.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>


/* This objects represents an open descriptor for an backend.
 * The backend has to save all information it needs in the \fd
 * member which is allocated with the appropriate size by this
 * generic API.
 */
struct io_object_t {
    signed int type;
    void *fd;
    ons_err_t err;
};

/* This list contains all currently open descriptors. */
MEM_ARRAY_DEFINE(io_glist_t, struct io_object_t*, 4, 1, 1, 0);
io_glist_t io_global_list;

/* Different backends.
 * They implement the different functions for a specific type.
 */
struct io_backend_t {
    /* Size of the \fd member in \io_object_t. */
    size_t size;
    /* Different function prototypes. Set to NULL if not supported. */
    unsigned int (*open)(struct io_object_t *obj);
    unsigned int (*merge)(struct io_object_t *obj);
    void (*close)(struct io_object_t *obj);
    unsigned int (*control)(struct io_object_t *obj, va_list *list);
    size_t (*write)(struct io_object_t *obj, const void *buf, size_t len);
    size_t (*read)(struct io_object_t *obj, void *buf, size_t len);
};

struct io_backend_t io_backends[IO_LAST] = {
    /* IO_UDP */
    {
        .size = 0,
        .open = NULL,
        .merge = NULL,
        .close = NULL,
        .control = NULL,
        .write = NULL,
        .read = NULL
    },
    /* IO_LAST */
    {
        .size = 0,
        .open = NULL,
        .merge = NULL,
        .close = NULL,
        .control = NULL,
        .write = NULL,
        .read = NULL
    }
};


/* Returns the next file descriptor number. */
static inline size_t io_next() {
    size_t i;
    for(i = 0; i < io_global_list.used; ++i) {
        if(io_global_list.list[i] == NULL) return i;
    }
    io_glist_t_push(&io_global_list);

    /* Do not return 0. */
    if(i == 0) return io_next();
    else return i;
}


/* Sets the value of an fd. */
static inline void io_set(io_fd_t fd, struct io_object_t *obj) {
    /* TODO: make threadsafe. */
    ONS_ASSERT(io_global_list.used < fd);

    io_global_list.list[fd] = obj;
}


/* Returns an appropriate io_object_t* for an integer. */
static inline struct io_object_t *io_obj(unsigned int fd) {
    /* TODO: make threadsafe. */
    if(fd >= io_global_list.used) return NULL;
    else return io_global_list.list[fd];
}


/* Returns the last error related to a file descriptor. */
ons_err_t io_err(io_fd_t fd) {
    struct io_object_t *obj;

    obj = io_obj(fd);
    if(!obj) return ONS_E_BADFD;
    else return obj->err;
}


/* Opens a new file descriptor. */
io_fd_t io_open(ons_err_t *err, signed int type, ...) {
    struct io_object_t *obj;
    va_list list;
    io_fd_t fd;

    if(type >= IO_LAST || !io_backends[type].open) {
        *err = ONS_E_BADTYPE;
        return IO_INVALID_FD;
    }

    obj = mem_zmalloc(sizeof(struct io_object_t) + io_backends[type].size);
    obj->type = type;
    obj->fd = sizeof(struct io_object_t) + (void*)obj;
    obj->err = ONS_E_NONE;

    va_start(list, type);
    if(!io_backends[type].open(obj, &list)) {
        *err = obj->err;
        mem_free(obj);
        return IO_INVALID_FD;
    }

    fd = io_next();
    io_set(fd, obj);
    return fd;
}


io_fd_t io_merge(signed int type, ...) {
    struct io_object_t *obj;
    va_list list;
    io_fd_t fd;

    if(type >= IO_LAST || !io_backends[type].merge) {
        *err = ONS_E_BADTYPE;
        return IO_INVALID_FD;
    }

    obj = mem_zmalloc(sizeof(struct io_object_t) + io_backends[type].size);
    obj->type = type;
    obj->fd = sizeof(struct io_object_t) + (void*)obj;
    obj->err = ONS_E_NONE;

    va_start(list, type);
    if(!io_backends[type].merge(obj, &list)) {
        *err = obj->err;
        mem_free(obj);
        return IO_INVALID_FD;
    }

    fd = io_next();
    io_set(fd, obj);
    return fd;
}


void io_close(io_fd_t fd) {
    struct io_object_t *obj;
    size_t i;

    obj = io_obj(fd);
    if(!obj) return;

    if(io_backends[obj->type].close) {
        io_backends[obj->type].close(obj);
    }

    mem_free(obj);
    io_set(fd, NULL);

    /* TODO: make threadsafe. */
    if(fd != io_global_list.used - 1) return;

    for(i = io_global_list.used - 1; i && !io_global_list.list[i]; --i) {
        io_glist_t_pop(&io_global_list);
    }
}


unsigned int io_control(io_fd_t fd, ...) {
    va_list list;
    struct io_object_t *obj;

    obj = io_obj(fd);
    if(!obj) return 0;
    else obj->err = ONS_E_NONE;

    if(io_backends[obj->type].control) {
        va_start(list, fd);
        return io_backends[obj->type].control(obj, &list);
    }

    obj->err = ONS_E_NOTSUPP;
    return 0;
}


size_t io_write(io_fd_t fd, ...) {
    va_list list;
    struct io_object_t *obj;

    obj = io_obj(fd);
    if(!obj) return 0;
    else obj->err = ONS_E_NONE;

    if(io_backends[obj->type].write) {
        va_start(list, fd);
        return io_backends[obj->type].write(obj, &list);
    }

    obj->err = ONS_E_NOTSUPP;
    return 0;
}


size_t io_read(io_fd_t fd, ...) {
    va_list list;
    struct io_object_t *obj;

    obj = io_obj(fd);
    if(!obj) return 0;
    else obj->err = ONS_E_NONE;

    if(io_backends[obj->type].read) {
        va_start(list, fd);
        return io_backends[obj->type].read(obj, &list);
    }

    obj->err = ONS_E_NOTSUPP;
    return 0;
}

