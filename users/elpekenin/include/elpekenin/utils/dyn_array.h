// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Dynamic (grows as needed) array of elements.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <errno.h>

#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"

/**
 * Metadata stored for a dynamic array.
 */
typedef struct PACKED {
    /**
     * The allocator it will use to grow.
     */
    allocator_t *allocator;

    /**
     * How many of slots the array has allocated.
     */
    size_t capacity;

    /**
     * How many slots are currently used.
     */
    size_t length;

    /**
     * Size of each element type, to allocate size accordingly.
     */
    size_t item_size;
} header_t;

/**
 * ----
 */

/**
 * Create a new dynamic array.
 *
 * Args:
 *     type: The type of the elements in the container.
 *     size: Initial size of the array. Useful to prevent extra allocation calls/wasted space.
 *     allocator: Used to grow.
 *
 * Return:
 *    A pointer to the first element in the newly created array.
 */
#define new_array(type, size, allocator) (type *)_new_array(sizeof(type), size, allocator)

/**
 * Internal function used by :c:macro:`new_array`.
 *
 * .. hint::
 *   You really shouldn't use this, but the macro that provides some convenience.
 */
READ_ONLY(3) void *_new_array(size_t item_size, size_t initial_size, allocator_t *allocator);

/**
 * Given a dynamic array, access its metadata.
 */
PURE READ_ONLY(1) static inline header_t *get_header(void *array) {
    if (UNLIKELY(array == NULL)) {
        return NULL;
    }

    return ((header_t *)array) - 1;
}

/**
 * Convenience to get the length of a dynamic array.
 */
PURE READ_ONLY(1) static inline size_t array_len(void *array) {
    header_t *header = get_header(array);

    if (UNLIKELY(header == NULL)) {
        return 0;
    }

    return header->length;
}

/**
 * Check if ``array`` is full (``capacity == length``). If so, make it bigger.
 *
 * Return: Error code.
 *    * ``0``: Size was enough, or could expand it.
 *    * ``-EINVAL``: ``array`` pointed to ``NULL``.
 *    * ``-ENOMEM``: Could not allocate required memory.
 *
 * .. attention::
 *   So far, this grow duplicates capacity, beware your memory usage and initialize
 *   the array with a better size if needed.
 */
WARN_UNUSED int expand_if_needed(void **array);

/**
 * Put a new element into an array.
 *
 * Return: Error code.
 *    * ``0``: Added it.
 *    * ``-EINVAL``: ```array`` pointed to ``NULL``.
 *    * ``-ENOMEM``: Could not allocate required memory.
 */
#define array_append(array, value)                                                 \
    ({                                                                             \
        /* unless we needed extra space and failed to allocate, append the item */ \
        int ret = expand_if_needed((void **)&array);                               \
        if (ret == 0) {                                                            \
            header_t *header = get_header(array);                                  \
            if (header == NULL) {                                                  \
                ret = -ENOMEM;                                                     \
            } else {                                                               \
                array[header->length++] = value;                                   \
            }                                                                      \
        }                                                                          \
                                                                                   \
        ret;                                                                       \
    })

/**
 * Remove ``n`` elements from the array.
 *
 * .. note::
 *   Does not zero out the memory, but mark it as "can be used".
 */
static inline void array_pop(void *array, size_t n) {
    header_t *header = get_header(array);
    header->length -= n;
}
