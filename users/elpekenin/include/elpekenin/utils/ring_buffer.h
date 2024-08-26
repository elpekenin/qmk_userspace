// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/util.h> // ARRAY_SIZE

// a ring buffer is a convenience wrapper for an array

#define new_rbuf(__type, __size, __name) \
    struct { \
        size_t next_in; \
        size_t next_out; \
        __type values[__size]; \
    } __name = {0}

#define rbuf_push(__buf, __value) \
    do { \
        __buf.values[__buf.next_in] = __value; \
        __buf.next_in = (__buf.next_in + 1) % ARRAY_SIZE(__buf.values); \
        if (__buf.next_in == __buf.next_out) { \
            __buf.next_out = (__buf.next_out + 1) % ARRAY_SIZE(__buf.values); \
        } \
    } while (0)

#define rbuf_pop(__buf, __max, __ptr) ({ \
    size_t i = 0; \
    size_t max = (__max == 0) ? rbuf_size(__buf) : __max; \
    while (i < max && __buf.next_out != __buf.next_in) { \
        __ptr[i++] = __buf.values[__buf.next_out]; \
        __buf.next_out = (__buf.next_out + 1) % ARRAY_SIZE(__buf.values); \
    } \
    /* bytes extracted */ \
    i; \
})

#define rbuf_clear(__buf) \
    do { \
        __buf.next_in = __buf.next_out = 0; \
    } while (0)

#define rbuf_size(__buf) \
    ARRAY_SIZE(__buf.values)

#define rbuf_full(__buf) \
    __buf.next_out == __buf.next_in - 1
