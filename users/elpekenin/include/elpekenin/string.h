// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Small string-related functions.
 */

// -- barrier --

#pragma once

#include "elpekenin/compiler.h"

/**
 * Write the value of ``n`` (# of bytes) in an human-friendly format, into ``buffer``
 *
 * .. hint::
 *   This operates inplace, but returns a pointer to the input for convenience.
 */
NON_NULL(2) RETURN_NO_NULL WRITE_ONLY(2, 3) const char *pretty_bytes(size_t n, char *buffer, uint16_t buffer_size);

/**
 * Check whether a char is UTF8.
 */
CONST bool is_utf8(char c);

/**
 * Check whether a char is a UTF8-continuation byte.
 */
CONST bool is_utf8_continuation(char c);
