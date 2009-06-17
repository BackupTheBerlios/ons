/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 26. May 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 30. May 2009
 */

/* UDP backend
 * UDP backend for the asynchio interface.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "asynchio/asynchio.h"
#include "backend.h"

#include <stdarg.h>
#include <stdint.h>


unsigned int asyn_udp_init(asyn_udp_t *udp, unsigned int opts, unsigned int type, const void *addr, unsigned int port) {
    unsigned int ret;

    SUNDRY_ASSERT(udp != NULL);

    /* Clear invalid options in \opts to be compatible to possible future
     * asynchio headers.
     */
    opts &= ASYN_UDP_NBLOCK | ASYN_UDP_CLOEXEC;

    /* First we need to create the UDP socket. We immediately set CLOEXEC if required because
     * some systems allow to set it directly in the socket() syscall.
     */
    ret = asyn_os_socket(&udp->fd, ASYN_OS_UDP, ASYN_IPV4, opts & ASYN_UDP_CLOEXEC);
    if(ret != ASYN_DONE) return ret;

    /* Now set the other options. */
    if(opts & ASYN_UDP_NBLOCK) {
        
    }
}

