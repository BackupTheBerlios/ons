/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. April 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 29. May 2009
 */

/* Defines miscellaneous asynchio functions
 * which are not related to any other group.
 */


#include "config/machine.h"
#include "sundry/sundry.h"
#include "asynchio/asynchio.h"
#include "memoria/memoria.h"
#include "backend.h"


#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>


#ifdef ONS_SOCKET_WIN_HEADERS
    #include <windows.h>
    #include <winsock2.h>
#endif
#ifdef ONS_SOCKET_BERKELEY_HEADERS
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif


unsigned int asyn_str2addr(unsigned int type, const char *str, void *addr) {
    char buffer[MEM_MAX(sizeof(struct in_addr), sizeof(struct in6_addr))];

    if(inet_pton((type == ASYN_IPV4)?AF_INET:AF_INET6, str, buffer) == 1) {
        if(type == ASYN_IPV4) {
            memcpy(addr, &((struct in_addr*)buffer)->s_addr, ASYN_V4SIZE);
        }
        else {
            memcpy(addr, ((struct in6_addr*)buffer)->s6_addr, ASYN_V6SIZE);
        }
        return ASYN_DONE;
    }
    else return ASYN_FAILED;
}


void asyn_addr2str(unsigned int type, const void *addr, char *str) {
    char buffer[MEM_MAX(sizeof(struct in_addr), sizeof(struct in6_addr))];

    if(type == ASYN_IPV4) {
        memcpy(&((struct in_addr*)buffer)->s_addr, addr, ASYN_V4SIZE);
    }
    else {
        memcpy(((struct in6_addr*)buffer)->s6_addr, addr, ASYN_V6SIZE);
    }

    if(inet_ntop((type == ASYN_IPV4)?AF_INET:AF_INET6, buffer, str, ASYN_STRLEN) == NULL) {
        /* inet_ntop should never fail. We supplied a buffer which is big enough
         * for the result and there is no other error which can occur.
         * However, just to go sure, we copy "0.0.0.0" or "::" into the buffer
         * to actually have a result.
         */
        if(type == ASYN_IPV4) {
            strncpy(str, "0.0.0.0", ASYN_STRLEN);
            str[ASYN_STRLEN - 1] = 0;
        }
        else {
            strncpy(str, "::", ASYN_STRLEN);
            str[ASYN_STRLEN - 1] = 0;
        }
        SUNDRY_DEBUG("asynchio: inet_ntop() returned NULL: errno (%d)", errno);
    }
}

