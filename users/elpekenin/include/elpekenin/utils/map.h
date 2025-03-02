// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Map strings (names) to values.
 *
 * .. note::
 *   Convenience wrapper on top of two dynamic arrays (:doc:`dyn_array`).
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"
#include "elpekenin/utils/shortcuts.h"

/**
 * Create a new map variable.
 *
 * Args:
 *     type: Of the values being stored.
 *     name: Of the map variable to be created.
 */
#define new_map(type, name)  \
    struct {                 \
        const char **keys;   \
        type        *values; \
    } name

/**
 * Initialize a ``map`` created by :c:macro:`new_map`. With a starting size and
 * an allocator.
 */
#define map_init(map, size, allocator)                                \
    do {                                                              \
        map.keys   = new_array(const char *, size, allocator);        \
        map.values = new_array(typeof(*map.values), size, allocator); \
    } while (0)

/**
 * Insert an element into a map.
 *
 * Args:
 *     map: Where to operate.
 *     key: Name of the element being added.
 *     value: Element added.
 *
 * .. attention::
 *   Does not overwrite existing elements
 *
 *   Does not handle errors when (if) :c:macro:`array_append` fails.
 *   Which, on the other hand, is very unlikely: no space left to allocate.
 */
#define map_set(map, key, value)                            \
    do {                                                    \
        /* if key already used, do nothing */               \
        int ret;                                            \
        WITHOUT_LOGGING(MAP, map_get(map, key, ret););      \
        if (ret == -ENOTFOUND) {                            \
            /* dont bother with value if key fails */       \
            if (array_append(map.keys, key) == 0) {         \
                /* if pushing value fails, pop key */       \
                if (array_append(map.values, value) != 0) { \
                    array_pop(map.keys, 1);                 \
                }                                           \
            }                                               \
        }                                                   \
    } while (0)

/**
 * Find an element by its key in a map.
 *
 * Args:
 *     map: Where to operate.
 *     key: Name of the element being searched.
 *     ret: Whether operation was successful. See :doc:`/headers/errno` for details.
 *
 * Return:
 *     The element identified by ``key``.
 *
 * .. danger::
 *   You **must** check ``ret`` before using the return value. If it was not found, you
 *   will just get the last element in the internal data structure.
 */
#define map_get(map, key, ret)                                      \
    ({                                                              \
        ret = -ENOTFOUND;                                           \
        size_t i;                                                   \
        for (i = 0; i < array_len(map.keys); ++i) {                 \
            if (map.keys[i] && strcmp(map.keys[i], key) == 0) {     \
                ret = 0;                                            \
                _   = logging(MAP, LOG_DEBUG, "Read '%s'", key);    \
                break; /* without this, we get one off (next ++) */ \
            }                                                       \
        }                                                           \
                                                                    \
        if (ret < 0) {                                              \
            logging(MAP, LOG_ERROR, "Key '%s' not found", key);     \
        }                                                           \
                                                                    \
        map.values[i];                                              \
    })
