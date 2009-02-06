/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 13. December 2007
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 26. December 2008
 */

/* Raw query interface.
 * Provides an API to contact a server and parse the response. There is no
 * interpretation of the reponse, though.
 */

#include <yaadns/misc.h>

#ifndef ONS_INCLUDED_yaadns_query_h
#define ONS_INCLUDED_yaadns_query_h
ONS_EXTERN_C_BEGIN

#include <memoria/memoria.h>

typedef struct yaadns_flags_t {
    uint8_t flag_qr : 1,
    uint8_t flags_opcode : 4,
    uint8_t flags_aa : 1,
    uint8_t flags_tc : 1,
    uint8_t flags_rd : 1,
    uint8_t flags_ra : 1,
    uint8_t flags_zero : 3,
    uint8_t flags_rcode : 4
} yaadns_flags_t;

typedef struct yaadns_raw_query_t {
    uint16_t q_id,
    yaadns_flags_t q_flags,
    uint16_t q_qdcount,
    uint16_t q_ancount,
    uint16_t q_nscount,
    uint16_t q_arcount,
    uint8_t q_payload[1]
} yaadns_query_t;

typedef struct yaadns_question_t {
    char *qname,
    uint16_t qtype,
    uint16_t qclass
} yaadns_question_t;

typedef struct yaadns_rr_t {
    char *qname,
    uint16_t qtype,
    uint16_t qclass,
    uint32_t ttl,
    uint16_t rdlength,
    uint8_t rdata[1]
} yaadns_rr_t;

MEM_ARRAY_DECLARE(yaadns_qdlist_t, yaadns_question_t, 1);
MEM_ARRAY_DECLARE(yaadns_rrlist_t, yaadns_rr_t, 1);

typedef struct yaadns_query_t {
    uint16_t q_id,
    yaadns_flags_t q_flags,
    yaadns_qdlist_t q_qdlist,
    yaadns_rrlist_t q_anlist,
    yaadns_rrlist_t q_nslist,
    yaadns_rrlist_t q_arlist
} yaadns_query_t;

extern bool yaadns_query_decode(char *);


ONS_EXTERN_C_END
#endif

