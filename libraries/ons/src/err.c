/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 7. February 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 24. February 2009
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
 * If \format is NULL, a default msg is printed.
 */
void ons_fatal_error(const char *format, ...) {
    va_list list;
    if(format) {
        va_start(list, format);
        vfprintf(stderr, format, list);
        va_end(list);
    }
    else fprintf(stderr, "ONS discovered a fatal error. No Error message specified; Exiting...");
    abort();
}

/* Function which is called on debug error.
 * If \format is NULL, a default msg is printed.
 */
void ons_debug_error(const char *format, ...) {
    va_list list;
    if(format) {
        va_start(list, format);
        vfprintf(stderr, format, list);
        va_end(list);
    }
    else fprintf(stderr, "ONS discovered a debug error. No error message specified; Ignoring...");
}

