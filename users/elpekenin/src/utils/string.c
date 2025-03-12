// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>
#include <string.h>

#include "elpekenin/utils/shortcuts.h"

bool is_utf8(char c) {
    return GET_BIT(c, 7); // 1xxx xxxx
}

bool is_utf8_continuation(char c) {
    return is_utf8(c) & !GET_BIT(c, 6); // 10xx xxxx
}

// returns pretty representation of an amount in bytes, eg: 8B or 1.3kB
const char *pretty_bytes(size_t n, char *buffer, int16_t buffer_size) {
    // bytes
    if (n < 1024) {
        // space for b to align with kb/mb/gb
        snprintf(buffer, buffer_size, "%3d b", n);
        return buffer;
    }

    const static char *magnitudes[] = {"kb", "mb", "gb"};

    int8_t index = 0;
    size_t copy  = n / 1024;
    while (copy >= 1024) {
        copy /= 1024;
        index++;
    }

    snprintf(buffer, buffer_size, "%3d%s", copy, magnitudes[index]);

    return buffer;
}
