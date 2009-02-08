/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 7. February 2009
 */

/* ONS error handling.
 * Provides functions for reliable error handling.
 */

#include "ons/ons.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Function which is called on fatal error.
 * Calls abort().
 * If \msg is NULL, a default msg is printed.
 */
void ons_fatal_error(const char *format, ...) {
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
    abort();
}

