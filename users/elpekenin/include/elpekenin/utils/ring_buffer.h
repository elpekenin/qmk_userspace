// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Implementation of a `ring buffer <https://en.wikipedia.org/wiki/Circular_buffer>`_.
 *
 * Tiny wrapper on top of an array.
 */

/**
 * ----
 */

// -- barrrier --

#pragma once

#include <quantum/util.h> // ARRAY_SIZE

/**
 * Create a new ring buffer.
 *
 * Args:
 *     type: Of the values being stored.
 *     size: Capacity of the buffer.
 *     name: Of the ring buffer variable to be created.
 */
#define new_rbuf(type, size, name) \
    struct {                       \
        size_t next_in;            \
        size_t next_out;           \
        type   values[size];       \
    } name = {0}

/**
 * Insert an element into a buffer.
 */
#define rbuf_push(rbuf, value)                                                    \
    do {                                                                          \
        rbuf.values[rbuf.next_in] = value;                                        \
        rbuf.next_in              = (rbuf.next_in + 1) % ARRAY_SIZE(rbuf.values); \
        if (rbuf.next_in == rbuf.next_out) {                                      \
            rbuf.next_out = (rbuf.next_out + 1) % ARRAY_SIZE(rbuf.values);        \
        }                                                                         \
    } while (0)

/**
 * Read data from a buffer.
 *
 * Args:
 *     rbuf: The ring buffer.
 *     max: Elements to be read at most. 0 -> Read everything
 *     dest: Memory where data is read.
 *
 * Return:
 *     Number of bytes read.
 */
#define rbuf_pop(rbuf, max, dest)                                          \
    ({                                                                     \
        size_t i   = 0;                                                    \
        size_t _max = (max == 0) ? rbuf_size(rbuf) : max;                   \
        while (i < _max && rbuf.next_out != rbuf.next_in) {                 \
            dest[i++]     = rbuf.values[rbuf.next_out];                    \
            rbuf.next_out = (rbuf.next_out + 1) % ARRAY_SIZE(rbuf.values); \
        }                                                                  \
        /* bytes extracted */                                              \
        i;                                                                 \
    })

/**
 * Mark a buffer as empty.
 *
 * .. note::
 *   Does not zero out the memory.
 */
#define rbuf_clear(rbuf)                  \
    do {                                  \
        rbuf.next_in = rbuf.next_out = 0; \
    } while (0)

/**
 * Get the size of a buffer.
 */
#define rbuf_size(rbuf) ARRAY_SIZE(rbuf.values)

/**
 * Check if the buffer is completely filled.
 */
#define rbuf_full(rbuf) rbuf.next_out == rbuf.next_in - 1
