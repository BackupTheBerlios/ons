/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 27. May 2009
 */

/* Generic File Descriptors
 * This API controls the file descriptors and describes the API
 * used by the backends. It delegates the work to the right
 * backend and checks for valid parameters.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "memoria/memoria.h"
#include "asynchio/asynchio.h"


#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>


/* Different backends.
 * They implement the different functions for a specific type.
 */
struct asyn_backend_t {
    /* Size of the \io member in \asyn_obj_t. */
    size_t size;
    /* Different function prototypes. Set to NULL if not supported. */
    unsigned int (*open)(asyn_obj_t *obj, unsigned int opts);
    unsigned int (*merge)(asyn_obj_t *obj, unsigned int opts, va_list list);
    void (*close)(asyn_obj_t *obj);
    signed int (*ctrl)(asyn_obj_t *obj, asyn_opt_t opt, va_list list);
    signed int (*write)(asyn_obj_t *obj, const void *buf, size_t *len);
    signed int (*read)(asyn_obj_t *obj, void *buf, size_t *len);
};

struct asyn_backend_t asyn_backends[ASYN_LAST + 1] = {
    /* ASYN_UDP */
    {
        /* .size = */ 0,
        /* .open = */ NULL,
        /* .merge = */ NULL,
        /* .close = */ NULL,
        /* .ctrl = */ NULL,
        /* .write = */ NULL,
        /* .read = */ NULL
    },
    /* ASYN_LAST */
    {
        /* .size = */ 0,
        /* .open = */ NULL,
        /* .merge = */ NULL,
        /* .close = */ NULL,
        /* .ctrl = */ NULL,
        /* .write = */ NULL,
        /* .read = */ NULL
    }
};

/* This calls the backend function. If the backend function is NULL, then obj->error is set appropriately
 * and (fail) is returned. \params must be the parameter list passed to the function encapsulated into
 * parentheses.
 * eg.: ASYN_BE_CALL(my_obj, ctrl, (my_obj, my_list), ASYN_FAILURE)
 */
#define ASYN_BE_CALL(obj, what, params, fail) (((obj)->type < ASYN_LAST && asyn_backends[(obj)->type].what) \
                    ?(asyn_backends[(obj)->type].what params) \
                    :(((obj)->error = ASYN_E_NOTSUPP), (fail)))


/* Library initialization.
 * We check that asyn_init() does only ONE initialization and increment an
 * internal counter to allow an application to call the init/deinit functions
 * while other libraries of the application also can call them without causing
 * any conflicts.
 */
static unsigned int asyn_init_count = 0;
unsigned int asyn_init() {
    unsigned int i;
    if(asyn_init_count++ == 0) {
        /* initialize */
        /* TODO */
    }
    else return ASYN_E_SUCCESS;
}
void asyn_deinit() {
    if(--asyn_init_count == 0) {
        /* deinitialize */
        /* TODO */
    }
}


unsigned int asyn_open(asyn_obj_t **obj, unsigned int type, unsigned int opts) {
    unsigned int ret;

    SUNDRY_ASSERT(obj != NULL);
    if(type >= ASYN_LAST) return ASYN_E_INVALTYPE;

    *obj = mem_zmalloc(sizeof(asyn_obj_t) + asyn_backends[type].size);
    (*obj)->io = sizeof(asyn_obj_t) + (void*)*obj;
    (*obj)->error = ASYN_E_SUCCESS;
    (*obj)->type = type;
    (*obj)->opts = ASYN_NONE;

    ret = ASYN_BE_CALL(*obj, open, (*obj, opts), (*obj)->error);
    if(ret != ASYN_E_SUCCESS) {
        mem_free(*obj);
        *obj = NULL;
    }
    return ret;
}


unsigned int asyn_merge(asyn_obj_t **obj, unsigned int type, unsigned int opts, ...) {
    va_list list;
    unsigned int ret;

    SUNDRY_ASSERT(obj != NULL);
    if(type >= ASYN_LAST) return ASYN_E_INVALTYPE;

    *obj = mem_zmalloc(sizeof(asyn_obj_t) + asyn_backends[type].size);
    (*obj)->io = sizeof(asyn_obj_t) + (void*)*obj;
    (*obj)->error = ASYN_E_SUCCESS;
    (*obj)->type = type;
    (*obj)->opts = opts;

    va_start(list, opts);
    ret = ASYN_BE_CALL(*obj, merge, (*obj, opts, list), (*obj)->error);
    if(ret != ASYN_E_SUCCESS) {
        mem_free(*obj);
        *obj = NULL;
    }
    va_end(list);
    return ret;
}


void asyn_close(asyn_obj_t *obj) {
    SUNDRY_ASSERT(obj != NULL);
    SUNDRY_MASSERT(obj->type < ASYN_LAST, "Invalid type in asynch-object.");
    ASYN_BE_CALL(obj, close, (obj), (void)0);
    mem_free(obj);
}


unsigned int asyn_err(asyn_obj_t *obj) {
    return obj->error;
}


signed int asyn_ctrl(asyn_obj_t *obj, asyn_opt_t opt, ...) {
    va_list list;
    signed int ret;

    SUNDRY_ASSERT(obj != NULL);
    SUNDRY_MASSERT(obj->type < ASYN_LAST, "Invalid type in asynch-object.");
    SUNDRY_MASSERT(opt & (ASYN_SET | ASYN_UNSET), "No SET/UNSET specified in option flag.");
    SUNDRY_MASSERT((opt & (ASYN_SET | ASYN_UNSET)) != (ASYN_SET | ASYN_UNSET), "Cannot SET and UNSET simultaneously.");

    va_start(list, opt);
    /* This may destroy \obj by calling asyn_close(). */
    ret = ASYN_BE_CALL(obj, ctrl, (obj, opt, list), ASYN_FAILURE);
    va_end(list);

    return ret;
}


signed int asyn_write(asyn_obj_t *obj, const void *buf, size_t *len) {
    SUNDRY_ASSERT(obj != NULL);
    SUNDRY_ASSERT(buf != NULL);
    SUNDRY_ASSERT(len != NULL);
    SUNDRY_MASSERT(obj->type < ASYN_LAST, "Invalid type in asynch-object.");

    /* This may destroy \obj by calling asyn_close(). */
    return ASYN_BE_CALL(obj, write, (obj, buf, len), ASYN_FAILURE);
}


signed int asyn_read(asyn_obj_t *obj, void *buf, size_t *len) {
    SUNDRY_ASSERT(obj != NULL);
    SUNDRY_ASSERT(buf != NULL);
    SUNDRY_ASSERT(len != NULL);
    SUNDRY_MASSERT(obj->type < ASYN_LAST, "Invalid type in asynch-object.");

    /* This may destroy \obj by calling asyn_close(). */
    return ASYN_BE_CALL(obj, read, (obj, buf, len), ASYN_FAILURE);
}

