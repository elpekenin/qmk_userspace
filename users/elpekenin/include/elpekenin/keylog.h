// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Utility to track last keys pressed in a string.
 *
 * This could later be shown on a screen, for example.
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>

#ifndef KEYLOG_SIZE
#    define KEYLOG_SIZE 40
#endif

/**
 * Hook into :c:func:`process_record_user` that performs the tracking.
 */
void keylog_process(uint16_t keycode, keyrecord_t *record);

/**
 * Read the current state of the keylog.
 */
const char *get_keylog(void);

/**
 * Whether anything has been written since keylog was last fetched.
 */
bool is_keylog_dirty(void);

/**
 * Takes a basic string representation of a keycode and
 * replace it with a prettier one. Eg: ``KC_A`` becomes ``A``
 */
void keycode_repr(const char **str);
