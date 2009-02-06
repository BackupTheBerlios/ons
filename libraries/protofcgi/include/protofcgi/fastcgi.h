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

/* Provides crucial information about a fastcgi process.
 * TODO
 */

#include <protofcgi/misc.h>

#ifndef ONS_INCLUDED_protofcgi_fastcgi_h
#define ONS_INCLUDED_protofcgi_fastcgi_h
ONS_EXTERN_C_BEGIN


#include <memoria/memoria.h>

/* Socket number where to listen for new connections of
 * FastCGI requests.
 * Is an FIP listener socket.
 */
#define FCGI_LISTENSOCKET ((fip_socket_t)0)

/* Returns true if the current process is FastCGI capable. Otherwise you should use normal CGI.
 * If you have a cgi_ctx_t structure, \fastcgi contains the return value and you don't need to call
 * this function, anymore.
 */
extern bool fcgi_is_fastcgi();

/* Returns pointer to the webserver's address. */
extern const char *fcgi_get_saddr();


ONS_EXTERN_C_END
#endif

