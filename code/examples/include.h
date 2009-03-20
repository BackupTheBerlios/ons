/*
 * (COPYRIGHT) Copyright (C) 2008, 2009, The ONS Team.
 * This file is part of ONS, see COPYING for details.
 */

/*
 * File information:
 * - Created: 20. March 2009
 * - Lead-Dev: - David Herrmann
 * - Contributors: /
 * - Last-Change: 20. March 2009
 */

/* This header is used by the examples. It contains stuff which
 * is used by more than one example but is only a small utility
 * which is used to show the functionality of the example and
 * avoiding uninteresting code.
 */


/* Calls a function (\func) several times (\times).
 * If \times is 0, \func is called indefinitely.
 */
static inline void sample_call(void (*func)(void), unsigned int times) {
    unsigned int i;
    for(i = 0; !times || i < times; ++i) {
        func();
    }
}

