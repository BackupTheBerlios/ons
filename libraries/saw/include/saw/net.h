/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 27. September 2008
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 1. January 2009
 */

/* netmask implementation
 * still TODO.
 */

#include <saw/misc.h>

#ifndef ONS_INCLUDED_saw_net_h
#define ONS_INCLUDED_saw_net_h
ONS_EXTERN_C_BEGIN


#if 0

/* Not implemented yet. */
/* some stuff: */
RFC 4291:
   2.3. Text Representation of Address Prefixes
   The text representation of IPv6 address prefixes is similar to the
   way IPv4 address prefixes are written in Classless Inter-Domain
   Routing (CIDR) notation [CIDR].  An IPv6 address prefix is
   represented by the notation:
      ipv6-address/prefix-length
   where
      ipv6-address    is an IPv6 address in any of the notations listed
                      in Section 2.2.
      prefix-length   is a decimal value specifying how many of the
                      leftmost contiguous bits of the address comprise
                      the prefix.


   The Subnet-Router anycast address is predefined.  Its format is as
   follows:
   |                         n bits                 |   128-n bits   |
   +------------------------------------------------+----------------+
   |                   subnet prefix                | 00000000000000 |
   +------------------------------------------------+----------------+
   The "subnet prefix" in an anycast address is the prefix that
   identifies a specific link.  This anycast address is syntactically
   the same as a unicast address for an interface on the link with the
   interface identifier set to zero.

#endif


ONS_EXTERN_C_END
#endif /* ONS_INCLUDED_saw_net_h */

