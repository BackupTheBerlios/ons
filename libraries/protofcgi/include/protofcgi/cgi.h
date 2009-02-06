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

/* CGI header. Wraps the CGI/1.1 and FastCGI/1.0 interfaces
 * in one simple API.
 * Both protocols are similar, even though they have to be
 * handled totally different. However, common CGI/1.1 formats
 * are also used in FastCGI/1.0.
 * This header provides all functions for using plain old
 * CGI/1.1 and, additionally, a wrapper for using CGI/1.1
 * and FastCGI/1.0 in a similar manner. Though, due to their
 * difference, the client programm still has to distinguish whether
 * it is a simple CGI program or FastCGI program.
 *
 * This header provides a function to allocate a cgi_ctx_t
 * context structure. The structure is either filled with
 * global FastCGI data or with global CGI data.
 */

#include <protofcgi/misc.h>

#ifndef ONS_INCLUDED_protofcgi_cgi_h
#define ONS_INCLUDED_protofcgi_cgi_h
ONS_EXTERN_C_BEGIN



/** CGI/1.1 Interface
 * Following, there are functions used in reading, parsing and writing CGI requests.
 * Most of the parsers can also be used in other cases than plain CGI/1.1. For instance
 * the form parser is used in FastCGI, too.
 * Furthermore, most of these functions take as argument either a string or an input
 * file to be able to parse STDIN directly. If you want to use CGI, you should not
 * use this low-level interface, instead you should look at the high-level wrapper
 * below this one. This has the nice side-effect that you can switch to FastCGI on-the-fly.
 */


/* All CGI/1.1 streams. */
#define CGI_STDIN ((fip_fd_t)0)
#define CGI_STDOUT ((fip_fd_t)0)
#define CGI_STDERR ((fip_fd_t)0)

/* Valid HTTP/1.1 methods. */
typedef enum cgi_method_t {
    CGI_METHOD_OPTIONS,
    CGI_METHOD_GET,
    CGI_METHOD_HEAD,
    CGI_METHOD_POST,
    CGI_METHOD_PUT,
    CGI_METHOD_DELETE,
    CGI_METHOD_TRACE,
    CGI_METHOD_CONNECT
} cgi_method_t;

/* Environment structure. */
typedef struct cgi_env_t {
    /* Points to the SERVER_SOFTWARE env-var. If not present, it points to an empty string.
     * eg., "Apache/2.2.11 (Unix) mod_ssl/2.2.11 OpenSSL/0.9.8j DAV/2"
     */
    const char *server_software;
    /* Points to a FQDN. If not present, it points to an empty string.
     * eg., "localhost" or "www.deutschland.de".
     */
    const char *server_name;
    /* Points to the webservers local address. Defaults to saw_addrx_default(). */
    saw_addr_t server_addr;
    /* Points to a string containing the gateway interface protocol version. If not present
     * it points to an empty string.
     * eg., "CGI/1.1"
     */
    const char *gateway_interface;
    /* Points to a string containing the server protocol version. If not present
     * it points to an empty string.
     * eg., "HTTP/1.1"
     */
    const char *server_protocol;
    /* Contains the server port. This defaults to 0. */
    saw_port_t server_port;
    /* Contains the request method, defaults to CGI_METHOD_GET. */
    cgi_method_t request_method;
    /* Points to a string containing the extra path information. If not available it is an emtpy string.
     * eg., http://localhost/index.cgi/test *CAN* call the index.cgi file and append "/test" as \path_info
     * if your webserver is configured for using PATH_INFO in this directory.
     */
    const char *path_info;
    /* Points to a string of a virtual-to-physical mapping og \path_info. If not available it is an empty string.
     * This is server-software dependent and not documented here.
     */
    const char *path_translated;
    /* Points to the virtual script being executed. If not present it is an empty string. This is *NOT* equal to the
     * path/file which is requested by the remote user. For example, if you use a rewrite engine this points to
     * the script which is returned by the rewrite engine.
     * It's a relative path which can be used for self-referencing.
     */
    const char *script_name;
    /* Contains the parsed query string. If no query string is available, this is NULL.
     * This is an array of cgi_pair_t structures which is terminated by an empty (key=NULL)
     * cgi_pair_t structure.
     */
    cgi_spair_t *query_string;
    /* Points to an FQDN of the remote user. If not present, it's an empty string.
     * eg., "localhost"
     */
    const char *remote_host;
    /* Points to the remote user's address. Defaults to saw_addrx_default(). */
    saw_addr_t remote_addr;
    /* If the server supports user authentication, and the script is protects,
     * this is the protocol-specific authentication method used to validate the user.
     * Points to an empty string if not available.
     */
    const char *auth_type;
    /* If the server supports user authentication, and the script is protected,
     * this is the username they have authenticated as.
     * If not available, this points to an empty string.
     */
    const char *remote_user;
    /* If the HTTP server supports RFC 931 identification, then this variable will
     * be set to the remote user name retrieved from the server. Usage of this variable
     * should be limited to logging only.
     * If not available, this points to an empty string.
     */
    const char *remote_ident;
    /* This is the content type of the data. If no content is available this points to
     * an empty string.
     */
    const char *content_type;
    /* This is the length of the content. Defaults to 0. */
    unsigned int content_length;
    /* Contains all non-standard HTTP_ variables. Terminated with an empty set (key=NULL). */
    cgi_cpair_t http[1];
} cgi_env_t;

/* Parses the environment variables into \env.
 * A cgi_ctx_t structure contains this, too. See the \env member.
 * Returns NULL on memory allocation errors.
 */
extern cgi_env_t *cgi_parse_env();

/* Frees an environment structure allocated through cgi_parse_env(). */
extern void cgi_free_env(cgi_env_t *env);

/* Parses \str as an application/x-www-form-urlencoded encoded string.
 * This mime-type is the most common one, future implementations may support
 * other formular-mime-types, but currently there is no other mime-type spread
 * in the internet.
 * Returns an array of cgi_pair_t structures. The End of the Array is indicated
 * with an empty cgi_pair_t structure.
 */
extern cgi_spair_t *cgi_parse_form(const char *str);
extern void cgi_free_form(cgi_spair_t *form);


/** Common Gateway Interface Wrapper
 * This wrapper allows to use CGI/1.1 and FastCGI/1.0 (and probably more protocols
 * in future) with the same interface. However, CGI processes do not keep running
 * between several requests, thus, you probably have to handle CGI and FastCGI
 * differently, because with FastCGI you could have a database directly in the CGI
 * parser process.
 *
 *   How to use this interface:
 * This interface creates first a CGI context (cgi_ctx_t) which identifies a CGI
 * process. You could use more than one of these ctx's in one single process,
 * however, this doesn't make sense in almost all cases.
 * Each ctx manages an indefinite amount of handlers where each handler can
 * also have an indefinite amount of requests assigned. When the ctx is created
 * it automatically has one handler, which is, however, not handled specially
 * compared to other handlers which can be created afterwards.
 * Now, when you created the context with the first handler, you can accept remote
 * requests with each handler. There is no need to balance the requests to all
 * handlers, moreover, you can create threads which manage the handlers. A single
 * handler should be used only in one single thread, though, several threads can
 * handle several requests simultaneously.
 * The handler registration and the context managing is not threadsafe, though.
 * The only threadsafe function is, obviously, cgi_accept() which assigns a new
 * request to an handler.
 * Freeing a ctx frees all handlers, freeing an handler stops all current requests
 * assigned to this handler.
 * When you accepted a request you can immediately start writing to it. However,
 * it is recommended to first 
 */

/* Forward declaration. */
struct cgi_ctx_t;
struct cgi_dump_t;
struct cgi_request_t;
struct cgi_task_t;

/* Array of file descriptors/handlers. */
MEM_ARRAY_DECLARE(cgi_dplist_t, cgi_dump_t, 1);
MEM_ARRAY_DECLARE(cgi_fdlist_t, fip_socket_t, 1);
MEM_ARRAY_DECLARE(cgi_rqlist_t, cgi_request_t, 1);

/* States of CGI structures. */
typedef enum cgi_states_t {
    CGI_NEGOTIATING,
#define CGI_NEGOTIATING ONS_BIT(CGI_NEGOTIATING)
    CGI_READ,
    CGI_WRITE,
    CGI_EXCEPTION,
    CGI_DEAD
} cgi_states_t;

/* CGI context structure. */
typedef struct cgi_ctx_t {
    bool fastcgi;                       /* Is FastCGI? */
    struct cgi_request_t *reqs[65536];  /* Array with all active requests. */
    cgi_dplist_t dlist;                 /* List of all dumps. */
    uint16_t max_conns;                 /* The maximum number of concurrent transport connections
                                         * this application will accept.
                                         */
    uint16_t max_reqs;                  /* The maximum number of concurrent requests this application will accept. */
    bool multiplex;                     /* True if this application multiplexes connections. */
    struct cgi_task_t task;             /* Most recent task. */
} cgi_ctx_t;

typedef struct cgi_dump_t {
    struct cgi_ctx_t *ctx;              /* Pointer to the parent context. */
    cgi_states_t state;                 /* Current state. */
    struct cgi_fdlist_t fdlist;         /* List of all managed FDs. */
    struct cgi_rqlist_t rqlist;         /* List of managed requests. */
    struct cgi_task_t task;             /* Most recent task. */
    char buffer[65536];                 /* Buffer used for receiving/sending. */
} cgi_handler_t;

typedef struct cgi_request_t {
    struct cgi_handler_t *handler;      /* Pointer to the parent handler. */
    cgi_states_t state;                 /* Current state. */
    uint16_t id;                        /* Request ID. */
    fip_socket_t fd;                    /* Related FD. */
    cgi_env_t evn;                      /* Environment variables. */
} cgi_request_t;

typedef enum cgi_tasktype_t {
    CGI_NONE = 0,           /* No task in queue. */
    CGI_SHUTDOWN,           /* CGI acceptor shutdown. */
    CGI_NEW_DUMP,           /* New dump. */
    CGI_NEW_REQ             /* New request. */
} cgi_tasktype_t;

typedef struct cgi_task_t {
    cgi_tasktype_t type;
    union cgi_taskunion_t {
        struct cgi_task_newdump_t {
            struct cgi_dump_t *dump;
        } new_dump;
        struct cgi_task_newreq_t {
            struct cgi_request_t *req;
        } new_req;
    } data;
} cgi_task_t;

extern cgi_ctx_t *cgi_init(uint16_t max_conns, uint16_t max_reqs, bool multiplex);
extern void cgi_free(cgi_ctx_t *ctx);
extern cgi_states_t cgi_listen(cgi_ctx_t *ctx);

extern cgi_task_t *cgi_accept(cgi_ctx_t *ctx);
extern void cgi_close(cgi_dump_t *dump);
extern const cgi_task_t *cgi_poll(cgi_dump_t *dump);

extern void cgi_attach(cgi_dump_t *dump);
extern void cgi_detach(cgi_dump_t *dump);


ONS_EXTERN_C_END
#endif

