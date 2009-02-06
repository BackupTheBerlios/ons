/*
 * (COPYRIGHT) Copyright (C) 2008, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 5. January 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 31. January 2009
 */

#include <protofcgi/protofcgi.h>
#include <stdlib.h>

/* - Accept connection.
 * - Check peer address for membership of FCGI_WEB_SERVER_ADDRS (comma separated list) and drop connection
 *   if not.
 * - Read fcgi records.
 * - Send 8byte aligned messages!
 * - Drop messages with inactive request IDs.
 * - Management records have a requestId value of zero, also called the null request ID. Application records have a nonzero requestId.
 * - A discrete record contains a meaningful unit of data all by itself. A stream record is part of a stream, i.e. a series
 *   of zero or more non-empty records (length != 0) of the stream type, followed by an empty record (length == 0) of the stream type.
 * - The Web server controls the lifetime of transport connections. The Web server can close a connection when no requests are active.
 *   Or the Web server can delegate close authority to the application (see FCGI_BEGIN_REQUEST). In this case the application closes
 *   the connection at the end of a specified request. 
 * - Initial: FCGI_GET_VALUES, FCGI_GET_VALUES_RESULT
 * - When an application receives a management record whose type T it does not understand,
 *   the application responds with {FCGI_UNKNOWN_TYPE, 0, {T}}.
 * - The Web server sends a FCGI_BEGIN_REQUEST record to start a request.
 * - Roles: FCGI_RESPONDER * FCGI_AUTHORIZER * FCGI_FILTER
 * - The Web server sends a FCGI_ABORT_REQUEST record to abort a request. After receiving {FCGI_ABORT_REQUEST, R}, the application responds
 *   as soon as possible with {FCGI_END_REQUEST, R, {FCGI_REQUEST_COMPLETE, appStatus}}. This is truly a response from the application,
 *   not a low-level acknowledgement from the FastCGI library. 
 * - The application sends a FCGI_END_REQUEST record to terminate a request, either because the application has processed the
 *   request or because the application has rejected the request.
 * - Use of the FCGI_STDERR stream is always optional.
 * - When a role protocol calls for transmitting a stream other than FCGI_STDERR, at least one record of the stream type is
 *   always transmitted, even if the stream is empty. 
 */

typedef struct {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
    unsigned char contentData[contentLength];
    unsigned char paddingData[paddingLength];
} FCGI_Record;

typedef struct {
    unsigned char nameLengthB3;  /* nameLengthB3  >> 7 == 1 */
    unsigned char nameLengthB2;
    unsigned char nameLengthB1;
    unsigned char nameLengthB0;
    unsigned char valueLengthB3; /* valueLengthB3 >> 7 == 1 */
    unsigned char valueLengthB2;
    unsigned char valueLengthB1;
    unsigned char valueLengthB0;
    unsigned char nameData[nameLength
            ((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
    unsigned char valueData[valueLength
            ((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
} FCGI_NameValuePair44;


const char *fcgi_get_saddr() {
    const char *ret;
    ret = getenv("FCGI_WEB_SERVER_ADDRS");
    if(!ret) return "";
    else return ret + 22;
}

bool fcgi_is_fastcgi() {
    saw_addr_t addr;
    fip_err_t err;

    if(!fip_optget(&err, FCGI_LISTENSOCKET, FIP_OPT_PEERNAME, &addr)) {
        if(err == FIP_E_NOTCONN) {
            return true;
        }
    }
    return false;
}

