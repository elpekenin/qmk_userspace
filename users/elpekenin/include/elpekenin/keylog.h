// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Utility to track last keys pressed in a string.
 *
 * This could later be shown on a screen, for example.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>

#include "elpekenin/utils/compiler.h"

/**
 * Hook into :c:func:`process_record_user` that performs the tracking.
 */
NON_NULL(2) READ_ONLY(2) void keylog_process(uint16_t keycode, keyrecord_t *record);

/**
 * Read the current state of the keylog.
 */
RETURN_NO_NULL const char *get_keylog(void);

/**
 * Whether anything has been written since keylog was last fetched.
 */
PURE bool is_keylog_dirty(void);

/**
 * Takes a basic string representation of a keycode and
 * replace it with a prettier one. Eg: ``KC_A`` becomes ``A``
 */
NON_NULL(1) void keycode_repr(const char **str);
