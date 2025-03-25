// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/string.h"

#include "elpekenin/shortcuts.h"
#include "printf.h"

bool is_utf8(char c) {
    return GET_BIT(c, 7); // 1xxx xxxx
}

bool is_utf8_continuation(char c) {
    return is_utf8(c) && !GET_BIT(c, 6); // 10xx xxxx
}

int pretty_bytes(string_t *str, size_t n) {
    // space for b to align with kb/mb/gb
    const static char *magnitudes[] = {" b", "kb", "mb", "gb"};

    int8_t index = 0;
    size_t copy  = n / 1024;
    while (copy >= 1024) {
        copy /= 1024;
        index++;
    }

    const int ret = snprintf(str->ptr, str->len, "%3d%s", (int)copy, magnitudes[index]);
    str->ptr += ret;
    str->len -= ret;
    return ret;
}

size_t lcpy(string_t *str, const char *literal) {
    const size_t ret = strlcpy(str->ptr, literal, str->len);
    str->ptr += ret;
    str->len -= ret;
    return ret;
}

size_t lcat(string_t *str, const char *literal) {
    const size_t ret = strlcat(str->ptr, literal, str->len);
    str->ptr += ret;
    str->len -= ret;
    return ret;
}
