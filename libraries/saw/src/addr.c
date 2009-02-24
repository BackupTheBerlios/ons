/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 19. April 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* Implementation of addr4.h, addr6.h, addrx.h, addrlo.h and addr.h.
 * See headers or function-comments for further information.
 * Internet Protocol address handling.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "saw/saw.h"

/* Old macros which no longer are needed. */
#define SAW_UINT(x) x
#define SAW_MKUINT(x) x

/* Implement the IPv4/6 default addresses here.
 * Declaring them as "static" would cause unnecessary memory usage.
 */
const saw_addr4_t saw_addr4_any = SAW_ADDR4_ANY;
const saw_addr4_t saw_addr4_loopback = SAW_ADDR4_LOOPBACK;
const saw_addr4_t saw_addr4_broadcast = SAW_ADDR4_BROADCAST;
const saw_addr6_t saw_addr6_any = SAW_ADDR6_ANY;
const saw_addr6_t saw_addr6_loopback = SAW_ADDR6_LOOPBACK;
const saw_addr6_t saw_addr6_interface_local = SAW_ADDR6_INTERFACE_LOCAL;
const saw_addr6_t saw_addr6_link_local = SAW_ADDR6_LINK_LOCAL;
const saw_addr6_t saw_addr6_interface_local_routers = SAW_ADDR6_INTERFACE_LOCAL_ROUTERS;
const saw_addr6_t saw_addr6_link_local_routers = SAW_ADDR6_LINK_LOCAL_ROUTERS;
const saw_addr6_t saw_addr6_site_local_routers = SAW_ADDR6_SITE_LOCAL_ROUTERS;

/* Converts an ASCII number into an unsigned int and vice versa. */
#define _saw_ctoi(x) (((char)(x)) - '0')
#define _saw_itoc(x) (((unsigned int)(x)) + '0')

/* Converts an hexadecimal character to an integer.
 * Returns zero if $c is an invalid character, thus, you must check it yourself with ctype's isxdigit()
 * function since '0' returns zero, too.
 */
static unsigned int _saw_hextoint(char c) {
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

/* Converts a null-terminated string holding an IPv4 address into a binary IPv4 address.
 *
 * IPv4 address syntax:
 * First: There is no official RFC about that like IPv6 has so we support here the most
 *        common styles, or, to be more honestly, all known styles.
 * type 1
 * Either: Four groups of a number between 0 and 255. Each group is separated with
 *         a dot. Each number can be represented decimal (no prefix), octal (0 prefix)
 *         or hexadecimal (case INsensitive and 0x prefix). Leading zeros BEHIND the
 *         prefix are allowed (decimal can't have leading zeros cause it would be parsed
 *         as octal). Example: 011.0.5.0x000Af => 9.0.5.115
 * type 2
 * Or: A dual system representation. Four octets separated by spaces. Each octet contains only
 *     zeros and ones and at most 8. Leading zeros can be omitted but no octet can be omitted.
 *     Example: "11000000 0000 0 10 00000001" => 192.0.2.1
 * type 3
 * Or: A number. This number can be decimal, octal or hexadecimal. Octal and decimal
 *     numbers are converted into hexadecimal and leading zeros are added until it is
 *     8 characters long (longer numbers are invalid). After that it is splittet into
 *     4 groups of 2 characters (string split) and parsed as dotted format described above.
 *     Example: Octal: 030000001353  Decimal: 3221226219
 *              Converted into hexadecimal (mathematic conversion): 0xC00002EB
 *              We do not need to add leading zeros due to it's length of 8 characters.
 *              Now convert into dotted notation by splitting: 0xC0.0x00.0x02.0xEB
 *              Convert into normal representation: 192.0.2.235
 *
 * Returns 0 on parse error and 1 on success.
 */
bool saw_addr4_pton(const char *ipstring, saw_addr4_t *dest) {
    unsigned int i, passed_digits, passed_spaces, passed_dots, ishexa, isocta, isdual;
    uint64_t value; /* To prevent overflows we need 64bit! */
    uint8_t *cdest = (uint8_t*)&dest->addr;

    /* Algorithm
     * We parse each char and check whether it is a valid character. If we encounter an invalid
     * character we stop parsing and return 0.
     * If the string ends without any parsed dot/space we handle it as a type 3 address as described above.
     * Otherwise, if we detect a dot we convert the first group into the destination and continue parsing.
     * If there are more or less than 4 groups we stop and return 0.
     * A single group is parsed by converting the last 3 characters into decimal and checking whether they
     * are greater than 255, if yes, we return 0, if not we check whether all characters before these three
     * are zeros. If not the number MUST be greater than 255 and we return 0, otherwise we save the value
     * and continue parsing the next group. Of course, only 3, 2 or 1 character(s) in a group is allowed, too.
     * But never an empty group!
     * A type 3 address is parsed like every other number, because we can simply convert it into dual system
     * and save it part by part.
     * Happy Addressing!
     */
    i = passed_digits = passed_spaces = passed_dots = ishexa = isocta = value = 0;
    /* everything is dual until we read a dot or an "x" or a digit greater than 1 or or or */
    isdual = 1;
    while(1) {
        if(ipstring[i] == '\0') {
            if(passed_digits == 0) return 0; /* empty string or group */
            else if(passed_dots > 0) {
                /* type 1 */
                if(passed_dots != 3) return 0;
                /* first convert the last 3 characters */
                if(ishexa) {
                    if(passed_digits < 3) return 0; /* hexa needs at least 0x0 */
                    passed_digits -= 2; /* remove prefix */
                    value = _saw_hextoint(ipstring[i - 1]);
                    if(passed_digits > 1) value += _saw_hextoint(ipstring[i - 2]) * 16;
                    /* If the 3rd character exists it has to be 0 because FF is already 255 */
                    if(passed_digits > 2 && ipstring[i - 3] != '0') return 0;
                    /* CAN'T be greater 255. */
                }
                else if(isocta && passed_digits != 1) {
                    if(passed_digits < 2) return 0; /* octa needs at least 01 if it is not 0 which is checked one line above. */
                    passed_digits -= 1; /* remove prefix */
                    value = _saw_ctoi(ipstring[i - 1]);
                    if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 8;
                    if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 64;
                    if(value > SAW_UINT(255)) return 0;
                }
                else {
                    value = _saw_ctoi(ipstring[i - 1]);
                    if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 10;
                    if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 100;
                    if(value > SAW_UINT(255)) return 0;
                }
                if(passed_digits > 3) {
                    /* Check whether the remaining characters are zeros. */
                    for(; passed_digits != 3; --passed_digits) if(ipstring[i - passed_digits] != '0') return 0;
                }
                /* Save the value. */
                cdest[passed_dots] = value;
                break;
            }
            else if(passed_spaces > 0) {
                /* type 2 */
                if(passed_spaces != 3) return 0;
                if(!isdual) return 0; /* Invalid character in type 2 address. */
                if(passed_digits > 8) return 0; /* empty/too big group */
                value = _saw_ctoi(ipstring[i - 1]);
                if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 2;
                if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 4;
                if(passed_digits > 3) value += _saw_ctoi(ipstring[i - 4]) * 8;
                if(passed_digits > 4) value += _saw_ctoi(ipstring[i - 5]) * 16;
                if(passed_digits > 5) value += _saw_ctoi(ipstring[i - 6]) * 32;
                if(passed_digits > 6) value += _saw_ctoi(ipstring[i - 7]) * 64;
                if(passed_digits > 7) value += _saw_ctoi(ipstring[i - 8]) * SAW_UINT(128);
                /* Save the value. */
                cdest[passed_spaces] = value;
                break;
            }
            else {
                /* type 3 */
                if(ishexa) {
                    if(passed_digits != 10) return 0; /* Hexa needs to be exactly 10 characters long (0xffFFffFF). */
                    cdest[0] = _saw_hextoint(ipstring[i - 7]) + _saw_hextoint(ipstring[i - 8]) * 16;
                    cdest[1] = _saw_hextoint(ipstring[i - 5]) + _saw_hextoint(ipstring[i - 6]) * 16;
                    cdest[2] = _saw_hextoint(ipstring[i - 3]) + _saw_hextoint(ipstring[i - 4]) * 16;
                    cdest[3] = _saw_hextoint(ipstring[i - 1]) + _saw_hextoint(ipstring[i - 2]) * 16;
                    break;
                }
                else if(isocta && passed_digits != 1) {
                    if(passed_digits < 2) return 0; /* octa needs at least 01 if it is not 0 which is checked one line above. */
                    passed_digits -= 1; /* remove prefix */
                    value = _saw_ctoi(ipstring[i - 1]);
                    if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) *                       8;
                    if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) *                      64;
                    if(passed_digits > 3) value += _saw_ctoi(ipstring[i - 4]) *            SAW_UINT(512);
                    if(passed_digits > 4) value += _saw_ctoi(ipstring[i - 5]) *           SAW_UINT(4096);
                    if(passed_digits > 5) value += _saw_ctoi(ipstring[i - 6]) *          SAW_UINT(32768);
                    if(passed_digits > 6) value += _saw_ctoi(ipstring[i - 7]) *       SAW_MKUINT(262144);
                    if(passed_digits > 7) value += _saw_ctoi(ipstring[i - 8]) *      SAW_MKUINT(2097152);
                    if(passed_digits > 8) value += _saw_ctoi(ipstring[i - 9]) *     SAW_MKUINT(16777216);
                    if(passed_digits > 9) value += _saw_ctoi(ipstring[i - 10]) *   SAW_MKUINT(134217728);
                    if(passed_digits > 10) value += _saw_ctoi(ipstring[i - 11]) * SAW_MKUINT(1073741824);
                    if(value > SAW_MKUINT(4294967295)) return 0; /* too big */
                    cdest[0] = (value & SAW_MKUINT(0xff000000)) >> 24;
                    cdest[1] = (value & SAW_MKUINT(0x00ff0000)) >> 16;
                    cdest[2] = (value & SAW_MKUINT(0x0000ff00)) >> 8;
                    cdest[3] = value & SAW_MKUINT(0x000000ff);
                    break;
                }
                else {
                    value = _saw_ctoi(ipstring[i - 1]);
                    if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) *                     10;
                    if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) *                    100;
                    if(passed_digits > 3) value += _saw_ctoi(ipstring[i - 4]) *          SAW_UINT(1000);
                    if(passed_digits > 4) value += _saw_ctoi(ipstring[i - 5]) *         SAW_UINT(10000);
                    if(passed_digits > 5) value += _saw_ctoi(ipstring[i - 6]) *        SAW_UINT(100000);
                    if(passed_digits > 6) value += _saw_ctoi(ipstring[i - 7]) *     SAW_MKUINT(1000000);
                    if(passed_digits > 7) value += _saw_ctoi(ipstring[i - 8]) *    SAW_MKUINT(10000000);
                    if(passed_digits > 8) value += _saw_ctoi(ipstring[i - 9]) *   SAW_MKUINT(100000000);
                    if(passed_digits > 9) value += _saw_ctoi(ipstring[i - 10]) * SAW_MKUINT(1000000000);
                    if(value > SAW_MKUINT(4294967295)) return 0; /* too big */
                    cdest[0] = (value & SAW_MKUINT(0xff000000)) >> 24;
                    cdest[1] = (value & SAW_MKUINT(0x00ff0000)) >> 16;
                    cdest[2] = (value & SAW_MKUINT(0x0000ff00)) >> 8;
                    cdest[3] = value & SAW_MKUINT(0x000000ff);
                    break;
                }
            }
        }
        else if(ipstring[i] == '.') {
            if(passed_digits == 0) return 0; /* empty group */
            if(passed_dots > 2) return 0; /* too much groups */
            /* first convert the last 3 characters */
            if(ishexa) {
                if(passed_digits < 3) return 0; /* hexa needs at least 0x0 */
                passed_digits -= 2; /* remove prefix */
                value = _saw_hextoint(ipstring[i - 1]);
                if(passed_digits > 1) value += _saw_hextoint(ipstring[i - 2]) * 16;
                /* If the 3rd character exists it has to be 0 because FF is already 255 */
                if(passed_digits > 2 && ipstring[i - 3] != '0') return 0;
                /* CAN'T be greater 255. */
            }
            else if(isocta && passed_digits != 1) {
                if(passed_digits < 2) return 0; /* octa needs at least 01 if it is not 0 which is checked one line above. */
                passed_digits -= 1; /* remove prefix */
                value = _saw_ctoi(ipstring[i - 1]);
                if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 8;
                if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 64;
                if(value > SAW_UINT(255)) return 0;
            }
            else {
                value = _saw_ctoi(ipstring[i - 1]);
                if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 10;
                if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 100;
                if(value > SAW_UINT(255)) return 0;
            }
            if(passed_digits > 3) {
                /* Check whether the remaining characters are zeros. */
                for(; passed_digits != 3; --passed_digits) if(ipstring[i - passed_digits] != '0') return 0;
            }
            /* Save the value. */
            cdest[passed_dots] = value;
            /* (re)set counters */
            passed_digits = 0;
            ++passed_dots;
            isdual = 0;
            ishexa = 0;
            isocta = 0;
        }
        else if(ipstring[i] == ' ') {
            if(!isdual) return 0; /* Invalid character in type 2 address. */
            if(passed_digits == 0 || passed_digits > 8) return 0; /* empty/too big group */
            if(passed_spaces > 2) return 0; /* too much groups */
            /* converter */
            value = _saw_ctoi(ipstring[i - 1]);
            if(passed_digits > 1) value += _saw_ctoi(ipstring[i - 2]) * 2;
            if(passed_digits > 2) value += _saw_ctoi(ipstring[i - 3]) * 4;
            if(passed_digits > 3) value += _saw_ctoi(ipstring[i - 4]) * 8;
            if(passed_digits > 4) value += _saw_ctoi(ipstring[i - 5]) * 16;
            if(passed_digits > 5) value += _saw_ctoi(ipstring[i - 6]) * 32;
            if(passed_digits > 6) value += _saw_ctoi(ipstring[i - 7]) * 64;
            if(passed_digits > 7) value += _saw_ctoi(ipstring[i - 8]) * SAW_UINT(128);
            /* Save the value. */
            cdest[passed_spaces] = value;
            /* (re)set counters */
            passed_digits = 0;
            ++passed_spaces;
            isocta = 0;
        }
        /* Check for hexadecimal prefix "0x". */
        else if(ipstring[i] == 'x') {
            /* isoctal has to be set and no other character has to be read to set ishexa. */
            if(passed_digits != 1 || !isocta) return 0; /* invalid character, at least at wrong position */
            ishexa = 1;
            ++passed_digits;
            isocta = 0;
            isdual = 0;
        }
        /* Check for octal prefix "0". */
        else if(ipstring[i] == '0') {
            /* If this is the first digit we have to set isoctal. */
            if(passed_digits == 0) isocta = 1;
            /* increment passed_digits anyway */
            ++passed_digits;
        }
        else if(isdigit(ipstring[i])) {
            /* octal does not allow digits greater than 7 and dual not greater than 1 */
            if(isocta && ipstring[i] > '7') return 0;
            if(ipstring[i] > '1') isdual = 0;
            ++passed_digits;
        }
        /* Now check for hexadecimal characters (a-fA-F).We checked already for digits so if _saw_hextoint()
           returns 0 the character is definitely no hexadecimal character. */
        else if(_saw_hextoint(ipstring[i]) != 0 && ishexa) {
            ++passed_digits;
            isdual = 0;
        }
        else return 0; /* invalid character */
        ++i;
    }
    return 1;
}

/* Creates a readable IPv4 string in dotted decimal notation.
 * Automatically sets the zero-terminator.  \pbuf must point to a buffer of
 * at least SAW_ADDR4_STRLEN size.
 *
 * Algorithm: The function splits the IPv4 address in 4 bytes. Each byte is divided first
 * by 100 and the result without the remainder is written to the buffer. The
 * remainder is divided by 10 and written too. The last remainder is the third
 * digit written in each group of the IPv4 address.
 */
void saw_addr4_ntop(const saw_addr4_t *ip, char *pbuf) {
    /* Returns the "y"th number of uint8_t-buffer "x". */
    #define _saw_get(x, y) (((const uint8_t*)(x))[(y)])

    unsigned int i;
    char *buf = pbuf;
    for(i = 0; i < 4; ++i) {
        *buf = _saw_itoc(_saw_get(&ip->addr, i) / 100);
        /* Prevent leading zeros => would be octal. */
        *((*buf == '0')?buf:++buf) = _saw_itoc(_saw_get(&ip->addr, i) % 100 / 10);
        *((*buf == '0' && buf == pbuf)?buf:++buf) = _saw_itoc(_saw_get(&ip->addr, i) % 100 % 10);
        *++buf = '.';
        ++buf;
        pbuf = buf;
    }
    *--buf = '\0';

    #undef _saw_get
}




/* Disable the following if IPv6 is disabled. */
#ifndef ONS_CONF_NO_IPV6




/* Reverted memory move
 * It is not *really* reverted, but this function moves the first \len bytes of \buf \dist
 * bytes into the right direction.  This do memcpy and strcpy too, but this function preserves
 * the bytes and starts copying at the end, thus, you can move 10bytes by 2:
 *   $buf="bla.abl0000" (length = 12 (including '\0'))
 *   saw_rev_move_(buf, 7, 4);
 *   $buf="bla.bla.abl"
 *   strncpy(buf, buf + 4, 7); or the similar memcpy call would result in the following one:
 *   $buf="bla.bla.bla";
 *
 * Helper function for saw_addr6_pton().
 */
static void _saw_rev_move(uint16_t *buf, unsigned int len, unsigned int dist) {
    signed int i;
    for(i = len - 1; i >= 0; --i) buf[i + dist] = buf[i];
}

/* Parses a zero-terminated IPv6 dotted notation string and saves the result in an
 * IPv6 address structure.
 *
 * The IPv6 address syntax is described in RFC 4291.
 *
 * Returns 0 on parse error and 1 on success.
 */
bool saw_addr6_pton(const char *ipstring, saw_addr6_t *dest) {
    unsigned int i, double_pos, passed_colons, value, passed_nums, req_i4;
    uint16_t *sdest = (uint16_t*)dest; /* one label is 16bit large */
    memset(sdest, 0, SAW_ADDR6_SIZE);

    /* If we find a double colon we save the position and proceed as it was a single one.
       At the end we move the tail back and fill the gap with zeros.
       We could also try to revert our direction and parse the address from the end if we discover
       a double colon, but I think the other way is faster.
       If we find an IPv4 address, we finish our parser and let saw_addr4_pton do the work. */
    i = double_pos = passed_colons = value = passed_nums = req_i4 = 0;
    while(1) {
        if(ipstring[i] == '\0') {
            if(req_i4) return 0; /* ipv4 is required */
            if(!double_pos) {
                if(passed_colons != 7 || passed_nums == 0) return 0; /* invalid number of colons or empty end */
            }
            else {
                /* If the last character was a double_colon we break our loop. */
                if(passed_nums == 0) {
                    if(passed_colons != double_pos) return 0; /* empty end */
                    else break;
                }
            }
            sdest[passed_colons] = ons_hton16(value);
            value = passed_nums = 0;
            ++passed_colons;
            break;
        }
        else if(ipstring[i] == ':') {
            if(req_i4) return 0; /* ipv4 is required */
            if(passed_colons == 7) return 0;
            else if(passed_nums == 0) {
                if(double_pos != 0) return 0; /* two double colons */
                if(passed_colons != 0) double_pos = ++passed_colons;
                else {
                    /* The first two characters were colons => assume the first
                       label were all zeros. */
                    if(ipstring[++i] == ':') {
                        sdest[0] = 0;
                        double_pos = passed_colons = 2;
                    }
                    else return 0; /* empty begin */
                }
                /* Set the gap to zero. */
                sdest[double_pos - 1] = 0;
            }
            else {
                sdest[passed_colons] = htons(value);
                value = passed_nums = 0;
                ++passed_colons;
            }
        }
        else if(ipstring[i] == '.') {
            /* The IPv4 address can only be at the end. */
            if(!double_pos && passed_colons != 6) return 0;
            else {
                if(passed_colons > 6) return 0;
                /* Move the tail to the end. */
                if(passed_colons != double_pos) _saw_rev_move(&sdest[double_pos], passed_colons - double_pos, 6 - passed_colons);
                /* Fill the gap with zeros. */
                memset(&sdest[double_pos], 0, (6 - passed_colons) * 2);
                /* Let the others do our work. */
                return saw_addr4_pton(&ipstring[i - passed_nums], (saw_addr4_t*)&sdest[6]);
            }
        }
        /* set \req_i4 if there are more than 4 hex numbers in one group */
        else if(passed_nums == 4 && ((req_i4 = 1), 0)) ;
        else if(isxdigit(ipstring[i])) {
            /* Received one digit, move the old ones to the left and add the new one. */
            value <<= 4;
            value += _saw_hextoint(ipstring[i]); /* Could be binary OR, too. */
            ++passed_nums;
        }
        else {
            req_i4 = 1; /* invalid character, may be a valid ipv4 one, set ipv4 to be required now! */
            ++passed_nums;
        }
        ++i;
    }

    /* Move tail and fill the gap. */
    if(double_pos) {
        if(passed_colons != double_pos) {
            _saw_rev_move(&sdest[double_pos], passed_colons - double_pos, 8 - passed_colons);
            memset(&sdest[double_pos], 0, (8 - passed_colons) * 2);
        }
        else memset(&sdest[double_pos - 1], 0, (7 - double_pos) * 2);
    }

    return 1;
}

/* Used for int2hex translation. */
static const char *_saw_hexadecimal = "0123456789abcdef";

/* Convert an IPv6 structure into a zero-terminated IPv6 address string. */
void saw_addr6_ntop(const saw_addr6_t *ip, char *ipstring) {
    unsigned int i, max_zeros, max_start, zeros, out;
    const uint16_t *sip = (const uint16_t*)ip;
    uint16_t label;

    /* return "::" if it all zero */
    if(ip->qword[0] == 0 && ip->qword[1] == 0) {
        memcpy(ipstring, "::", 3);
        return;
    }

    /* Algorithm from Michael Poole (Entrope). */
    /* Find longest run of zeros. */
    i = max_zeros = max_start = zeros = 0;
    for(; i < 8; ++i) {
        if(ons_ntoh16(sip[i]) == 0) ++zeros;
        else if(zeros > max_zeros) {
            max_zeros = zeros;
            max_start = i - zeros + 1;
            zeros = 0;
        }
    }
    if(zeros > max_zeros) {
        max_zeros = zeros;
        max_start = i - zeros;
    }

    /* 'Cause max_start is zero, if no zero is found, max_start == 1 means we start
       with a double colon. */
    out = 0;
    if(max_start == 1) ipstring[out++] = ':';
    for(i = 0; i < 8; ++i) {
        if(i + 1 == max_start) {
            ipstring[out++] = ':';
            i += max_zeros;
            --i;
        }
        else {
            /* Simple bitwise addition. */
            label = htons(sip[i]);
            if(label >= SAW_UINT(0x1000)) ipstring[out++] = _saw_hexadecimal[(label >> 12) & 0xf];
            if(label >= SAW_UINT(0x100)) ipstring[out++] = _saw_hexadecimal[(label >> 8) & 0xf];
            if(label >= 0x10) ipstring[out++] = _saw_hexadecimal[(label >> 4) & 0xf];
            ipstring[out++] = _saw_hexadecimal[label & 0xf];
            ipstring[out++] = ':';
        }
    }
    /* Reset last colon to zero. */
    ipstring[out - 1] = '\0';
}




#endif /* #ifndef ONS_CONF_NO_IPV6 */


