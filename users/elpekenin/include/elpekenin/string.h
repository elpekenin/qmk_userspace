// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Small string-related functions.
 */

// -- barrier --

#pragma once

#include <stdint.h>
#include <string.h> // srlen

/**
 * Check whether a char is UTF8.
 */
bool is_utf8(char c);

/**
 * Check whether a char is a UTF8-continuation byte.
 */
bool is_utf8_continuation(char c);

typedef struct {
    char  *ptr;
    size_t len;
} string_t;

static inline string_t new_string(size_t n) {
    char buf[n];
    return (string_t){
        .ptr = buf,
        .len = n,
    };
}

/**
 * Write the value of ``n`` (# of bytes) in an human-friendly format, into ``string``
 */
int pretty_bytes(string_t *str, size_t n);

size_t lcpy(string_t *str, const char *literal);
size_t lcat(string_t *str, const char *literal);
