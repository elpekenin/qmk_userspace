// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Small string-related functions.
 */

// -- barrier --

#pragma once

/**
 * Write the value of ``n`` (# of bytes) in an human-friendly format, into ``buffer``
 *
 * .. hint::
 *   This operates inplace, but returns a pointer to the input for convenience.
 */
const char *pretty_bytes(size_t n, char *buffer, uint16_t buffer_size);

/**
 * Check whether a char is UTF8.
 */
bool is_utf8(char c);

/**
 * Check whether a char is a UTF8-continuation byte.
 */
bool is_utf8_continuation(char c);
