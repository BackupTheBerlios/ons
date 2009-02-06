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
#include <memoria/memoria.h>
#include <saw/saw.h>
#include <fip/fip.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef ONS_CONF_HAVE_UNISTD_H
    #include <unistd.h>
#endif

MEM_ARRAY_DECLARE(cgi_table_http_t, char *, 4);

cgi_env_t *cgi_parse_env() {
    cgi_env_t env, *ret;
    cgi_table_http_t env_http;
    unsigned int i, sum;
    char c, **ele, *str;
    void *mem;

    cgi_table_http_t_init(&env_http);
    memset(&env, 0, sizeof(env));
    saw_addrx_default(&env.server_addr.in);
    env.request_method = CGI_METHOD_GET;
    saw_addrx_default(&env.remote_addr.in);

    /* SUSv3 defines the names (not keys) to be ASCII encoded. */
    sum = 0;
    for(i = 0; environ[i]; ++i) {
        c = environ[i][0];

        switch(c) {
            case 'A':
                if(strncmp(environ[i], "AUTH_TYPE=", 10) == 0) {
                    env.auth_type = environ[i] + 10;
                }
                break;
            case 'B':
                break;
            case 'C':
                if(strncmp(environ[i], "CONTENT_TYPE=", 13) == 0) {
                    env.content_type = environ[i] + 13;
                }
                else if(strncmp(environ[i], "CONTENT_LENGTH=", 15) == 0) {
                    str = environ[i] + 15;
                    env.content_length = atoi(str);
                }
                break;
            case 'D':
                break;
            case 'E':
                break;
            case 'F':
                break;
            case 'G':
                if(strncmp(environ[i], "GATEWAY_INTERFACE=", 18) == 0) {
                    env.gateway_interface = environ[i] + 18;
                }
                break;
            case 'H':
                if(strncmp(environ[i], "HTTP_", 5) == 0 && environ[i][5] != '=') {
                    ele = cgi_table_http_t_push(&env_http);
                    if(!ele) {
                        cgi_table_http_t_clean(&env_http);
                        return NULL;
                    }
                    *ele = environ[i];
                    /* Minus the equal sign, plus 2 times the \0 => + 1 */
                    sum += strlen(environ[i]) + 1;
                }
                break;
            case 'I':
                break;
            case 'J':
                break;
            case 'K':
                break;
            case 'L':
                break;
            case 'M':
                break;
            case 'N':
                break;
            case 'O':
                break;
            case 'P':
                if(strncmp(environ[i], "PATH_INFO=", 10) == 0) {
                    env.path_info = environ[i] + 10;
                }
                else if(strncmp(environ[i], "PATH_TRANSLATED=", 16) == 0) {
                    env.path_translated = environ[i] + 16;
                }
                break;
            case 'Q':
                if(strncmp(environ[i], "QUERY_STRING=", 13) == 0) {
                    str = environ[i] + 13;
                    env.query_string = cgi_parse_form(str);
                    /* Ignore memory errors here. */
                }
                break;
            case 'R':
                if(strncmp(environ[i], "REMOTE_USER=", 12) == 0) {
                    env.remote_user = environ[i] + 12;
                }
                else if(strncmp(environ[i], "REMOTE_ADDR=", 12) == 0) {
                    str = environ[i] + 12;
                    if(!saw_addrx_pton(str, &env.remote_addr.in)) saw_addrx_default(&env.remote_addr.in);
                }
                else if(strncmp(environ[i], "REMOTE_HOST=", 12) == 0) {
                    env.remote_host = environ[i] + 12;
                }
                else if(strncmp(environ[i], "REMOTE_IDENT=", 13) == 0) {
                    env.remote_ident = environ[i] + 13;
                }
                else if(strncmp(environ[i], "REQUEST_METHOD=", 15) == 0) {
                    str = environ[i] + 15;
                    if(strcmp(str, "GET") == 0) env.request_method = CGI_METHOD_GET;
                    else if(strcmp(str, "POST") == 0) env.request_method = CGI_METHOD_POST;
                    else if(strcmp(str, "PUT") == 0) env.request_method = CGI_METHOD_PUT;
                    else if(strcmp(str, "HEAD") == 0) env.request_method = CGI_METHOD_HEAD;
                    else if(strcmp(str, "OPTIONS") == 0) env.request_method = CGI_METHOD_OPTIONS;
                    else if(strcmp(str, "DELETE") == 0) env.request_method = CGI_METHOD_DELETE;
                    else if(strcmp(str, "TRACE") == 0) env.request_method = CGI_METHOD_TRACE;
                    else if(strcmp(str, "CONNECT") == 0) env.request_method = CGI_METHOD_CONNECT;
                }
                break;
            case 'S':
                if(strncmp(environ[i], "SCRIPT_NAME=", 12) == 0) {
                    env.script_name = environ[i] + 12;
                }
                else if(strncmp(environ[i], "SERVER_PORT=", 12) == 0) {
                    str = environ[i] + 12;
                    env.server_port = atoi(str);
                }
                else if(strncmp(environ[i], "SERVER_NAME=", 12) == 0) {
                    env.server_name = environ[i] + 12;
                }
                else if(strncmp(environ[i], "SERVER_ADDR=", 12) == 0) {
                    str = environ[i] + 12;
                    if(!saw_addrx_pton(str, &env.server_addr.in)) saw_addrx_default(&env.server_addr.in);
                }
                else if(strncmp(environ[i], "SERVER_PROTOCOL=", 16) == 0) {
                    env.server_protocol = environ[i] + 16;
                }
                else if(strncmp(environ[i], "SERVER_SOFTWARE=", 16) == 0) {
                    env.server_software = environ[i] + 16;
                }
                break;
            case 'T':
                break;
            case 'U':
                break;
            case 'V':
                break;
            case 'W':
                break;
            case 'X':
                break;
            case 'Y':
                break;
            case 'Z':
                break;
        }
    }

    if(!(ret = mem_malloc(sizeof(cgi_env_t) + env_http.used * sizeof(cgi_pair_t) + sum))) {
        cgi_table_http_t_clean(&env_http);
        return NULL;
    }

    if(env.server_software) ret->server_software = env.server_software;
    else ret->server_software = "";

    if(env.server_name) ret->server_name = env.server_name;
    else ret->server_name = "";

    saw_addr_cpy(&env.server_addr, &ret->server_addr);

    if(env.gateway_interface) ret->gateway_interface = env.gateway_interface;
    else ret->gateway_interface = "";

    if(env.server_protocol) ret->server_protocol = env.server_protocol;
    else ret->server_protocol = "";

    ret->server_port = env.server_port;

    ret->request_method = env.request_method;

    if(env.path_info) ret->path_info = env.path_info;
    else ret->path_info = "";

    if(env.path_translated) ret->path_translated = env.path_translated;
    else ret->path_translated = "";

    if(env.script_name) ret->script_name = env.script_name;
    else ret->script_name = "";

    ret->query_string = env.query_string;

    if(env.remote_host) ret->remote_host = env.remote_host;
    else ret->remote_host = "";

    saw_addr_cpy(&env.remote_addr, &ret->remote_addr);

    if(env.auth_type) ret->auth_type = env.auth_type;
    else ret->auth_type = "";

    if(env.remote_user) ret->remote_user = env.remote_user;
    else ret->remote_user = "";

    if(env.remote_ident) ret->remote_ident = env.remote_ident;
    else ret->remote_ident = "";

    if(env.content_type) ret->content_type = env.content_type;
    else ret->content_type = "";

    ret->content_length = env.content_length;

    /* Set \mem to the memory behind the \http array in \ret. */
    mem = sizeof(cgi_env_t) + (void*)ret;
    mem += env_http.used * sizeof(cgi_pair_t);
    for(i = 0; i < env_http.used; ++i) {
        /* Save http list in memory.
         * The whole number of key-value pairs is env_http.used + 1. However, the
         * first element is part of the structure itself, thus env_http elements
         * lie in the memory beyond the structure. Behind these elements, there is
         * space for the strings. Each key-value pair is saved together in memory.
         * though, the equal sign is not stored.
         */
        str = env_http.list[i];
        ret->http[i].key = mem;
        while(*str != '=' && (*(char*)mem++ = *str++)) /* empty loop */ ;
        *(char*)mem++ = '\0';
        ++str;

        ret->http[i].value = mem;
        while((*(char*)mem++ = *str++)) /* empty loop */ ;
    }
    ret->http[i].key = NULL;
    ret->http[i].value = NULL;

    cgi_table_http_t_clean(&env_http);
    return ret;
}

void cgi_free_env(cgi_env_t *env) {
    if(env->query_string) cgi_free_form(env->query_string);
    mem_free(env);
}

/* Parses \str as an application/x-www-form-urlencoded encoded string.
 * Returns an array of cgi_pair_t structures. The End of the Array is indicated
 * with an empty cgi_pair_t structure.
 * All binary \0s are removed inside a key or value.
 *
 * cgi_escp_copy() is only a helper function for cgi_parse_form().
 */

MEM_ARRAY_DECLARE(cgi_www_form_t, cgi_pair_t, 2);

/* Converts an hexadecimal character to an integer.
 * Returns zero if $c is an invalid character, thus, you must check it yourself with ctype's isxdigit()
 * function since '0' returns zero, too.
 */
static unsigned int cgi_hextoint(char c) {
    switch(c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a':
        case 'A': return 10;
        case 'b':
        case 'B': return 11;
        case 'c':
        case 'C': return 12;
        case 'd':
        case 'D': return 13;
        case 'e':
        case 'E': return 14;
        case 'f':
        case 'F': return 15;
        default: return 0;
    }
}

/* We prevent every zero character from being copied into \dest except the terminating \0.
 * See comment above cgi_hextoint().
 * \len contains the maximum length of \dest.
 * Stops either if \len is reached or '&' or '\0' is read.
 * Returns the length of \dest without the written \0 character.
 */
static size_t cgi_escp_copy(char *dest, const char *source, size_t len) {
    size_t i, out;
    bool escp, parsed;

    escp = false;
    parsed = false;
    out = 0;
    for(i = 0; out < len; ++i) {
        switch(source[i]) {
            case '+':
                if(escp) {
                    /* Invalid escape sequence. Check whether at least one number was parse (eg., "%1")
                     * and convert it as hexadecimal value, otherwise (single "%") we just copy the "%".
                     */
                    if(parsed) dest[out] = cgi_hextoint(source[i - 1]);
                    else dest[out] = '%';
                    escp = false;
                    parsed = false;
                    ++out;
                }
                dest[out] = ' ';
                break;
            case '%':
                if(escp) {
                    /* Invalid escape sequence. Check whether at least one number was parse (eg., "%1")
                     * and convert it as hexadecimal value, otherwise we just copy the "%" and restart
                     * the escape sequence.
                     */
                    if(parsed) dest[out] = cgi_hextoint(source[i - 1]);
                    else dest[out] = '%';
                    parsed = false;
                    ++out;
                }
                else escp = 1;
                break;
            case '&':
            case '\0':
                if(escp) {
                    /* Invalid escape sequence. Check whether at least one number was parse (eg., "%1")
                     * and convert it as hexadecimal value, otherwise (single "%") we just copy the "%".
                     */
                    if(parsed) dest[out] = cgi_hextoint(source[i - 1]);
                    else dest[out] = '%';
                    escp = false;
                    parsed = false;
                    ++out;
                }
                dest[out] = '\0';
                return out;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'A':
            case 'b':
            case 'B':
            case 'c':
            case 'C':
            case 'd':
            case 'D':
            case 'e':
            case 'E':
            case 'f':
            case 'F':
                if(escp) {
                    if(parsed) {
                        dest[out] = (cgi_hextoint(source[i - 1]) << 4) | cgi_hextoint(source[i]);
                        escp = false;
                        parsed = false;
                    }
                    else parsed = true;
                }
                else dest[out] = source[i];
                break;
            default:
                /* If we encounter something else than a number in an escape sequence, we just assume that the
                 * '%' was not escaped and copy it. If only the second number is missing we just assume the escape
                 * sequence has only one character.
                 */
                if(escp) {
                    if(parsed) dest[out] = cgi_hextoint(source[i - 1]);
                    else dest[out] = '%';
                    escp = false;
                    parsed = false;
                    ++out;
                }
                /* This is a special char, though, it has to be escaped as explained in the HTML standard.
                 * We simply copy the special char.
                 */
                dest[out] = source[i];
        }

        /* We remove all binary \0's in the string. */
        ++out;
    }
    dest[len - 1] = '\0';
    return out - 1;
}

/* see comment above cgi_escp_copy() */
cgi_spair_t *cgi_parse_form(const char *str) {
    cgi_www_form_t arr;
    cgi_pair_t *ele;
    size_t i, type, klen, vlen;
    const char *key, *value;

    cgi_www_form_t_init(&arr);
    type = 0; /* 0 == key parsing; 1 == value parsing; */
    klen = 0; /* len of the current key */
    vlen = 0; /* len of the current value */
    key = value = str;

    for(i = 0; true; ++i) {
        switch(str[i]) {
            case '=':
                /* If '=' is a character in a value we just assume someone missed to
                 * escape it.
                 */
                if(type == 0) {
                    type = 1;
                    value = &str[i] + 1;
                }
                break;
            case '&': /* fallthrough */
            case '\0':
                if(klen == 0) {
                    key = &str[i] + 1;
                    klen = 0;
                    vlen = 0;
                    type = 0;
                    break;
                }
                ele = cgi_www_form_t_push(&arr);
                if(!ele) goto mem_fail;
                ele->key = mem_malloc(klen + vlen + 2);
                if(!ele->key) {
                    /* Prevent it from being freed below. */
                    ele->key = NULL;
                    goto mem_fail;
                }
                ele->value = klen + 1 + (void*)ele->key;
                ele->klen = cgi_escp_copy(ele->key, key, klen + 1);
                if(vlen) ele->vlen = cgi_escp_copy(ele->value, value, vlen + 1);
                else ele->value[0] = '\0';
                key = &str[i] + 1;
                klen = 0;
                vlen = 0;
                type = 0;
                break;
            default:
                if(type == 0) ++klen;
                else ++vlen;
                break;
        }
        if(!str[i]) break;
    }

    /* Return the pointer to the array.
     * The wrapper around this pointer is created on the stack
     * and, thus, deleted automatically.
     */
    ele = cgi_www_form_t_push(&arr);
    if(!ele) goto mem_fail;
    ele->key = NULL;
    ele->value = NULL;
    return arr.list;

    /* Delete the array. */
    mem_fail:
    for(i = 0; i < arr.used; ++i) mem_free(arr.list[i].key);
    cgi_www_form_t_clean(&arr);
    return NULL;
}

void cgi_free_form(cgi_pair_t *form) {
    cgi_pair_t *iter;
    for(iter = form; iter->key; ++iter) mem_free(iter->key);
    mem_free(form);
}



/*********************************************************************************************/
/*********************************************************************************************/
/***********                                                                       ***********/
/***********                  Common Gateway Interface Wrapper                     ***********/
/***********                                                                       ***********/
/*********************************************************************************************/
/*********************************************************************************************/



/* Initializes a new CGI context structure.
 * \max_conns and \max_reqs have to be at least 1. If they are 0, they are set
 * automatically to 1.
 */
cgi_ctx_t *cgi_init(uint16_t max_conns, uint16_t max_reqs, bool multiplex) {
    cgi_ctx_t *ctx;

    if(!(ctx = mem_malloc(sizeof(cgi_ctx_t)))) return NULL;

    if(max_conns == 0) max_conns = 1;
    if(max_reqs == 0) max_reqs = 1;

    memset(ctx, 0, sizeof(*ctx));
    ctx->fastcgi = fcgi_is_fastcgi();
    cgi_dplist_t_init(&ctx->dlist);
    ctx->max_conns = max_conns;
    ctx->max_reqs = max_reqs;
    ctx->multiplex = multiplex;

    return ctx;
}

void cgi_free(cgi_ctx_t *ctx) {
    /* TODO */
}

/* Returns the states we need to be notified for on the FCGI_LISTENSOCKET.
 * If the returned bitset is empty, we do not expect any notifications.
 */
cgi_states_t cgi_listen(cgi_ctx_t *ctx) {
    if(ctx->fastcgi) return CGI_READ;
    else return 0;
}

/* Accepts a new task on \ctx.
 * Returns NULL if memory allocation failed.
 */
cgi_task_t *cgi_accept(cgi_ctx_t *ctx) {
    cgi_dump_t *dump;
    bool new_dump = false;
    fip_socket_t fd;
    fip_err_t err;
    saw_addr_t addr;

    /* Following tasks can occur on the main ctx structure:
     * - CGI_NONE
     * - CGI_SHUTDOWN
     * - CGI_NEW_DUMP
     */

    /* CGI/1.1 initiates shutdown if this function is called more than once. */
    if(!ctx->fastcgi) {
        /* CGI/1.1 allows at most 1 connection. */
        if(ctx->dlist.used > 0) {
            ctx->task.type = CGI_SHUTDOWN;
            return &ctx->task;
        }
        new_dump = true;
    }
    else {
        fd = fip_accept(&err, FCGI_LISTENSOCKET, &addr);
        
    }

    if(!new_dump) {
        /* No task. */
        ctx->task.type = CGI_NONE;
        return &ctx->task;
    }

    /* Return new dump. */
    if(!(dump = mem_alloc(sizeof(cgi_dump_t)))) return NULL;
}

typedef struct cgi_dump_t {
    struct cgi_ctx_t *ctx;              /* Pointer to the parent context. */
    cgi_states_t state;                 /* Current state. */
    struct cgi_fdlist_t fdlist;         /* List of all managed FDs. */
    struct cgi_rqlist_t rqlist;         /* List of managed requests. */
    struct cgi_task_t task;             /* Most recent task. */
    char buffer[65536];                 /* Buffer used for receiving/sending. */
} cgi_handler_t;

void cgi_close(cgi_dump_t *dump);
const cgi_task_t *cgi_poll(cgi_dump_t *dump);
void cgi_attach(cgi_dump_t *dump);
void cgi_detach(cgi_dump_t *dump);

