// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/errno.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"

typedef struct PACKED {
    allocator_t *allocator;
    size_t       capacity;  // allocated slots
    size_t       length;    // filled slots
    size_t       item_size;
} header_t;

#define new_array(__type, __size, __allocator) (__type *)_new_array(sizeof(__type), __size, __allocator)
READ_ONLY(3) void *_new_array(size_t item_size, size_t initial_size, allocator_t *allocator);


PURE READ_ONLY(1) static inline header_t *get_header(void *array) {
    if (UNLIKELY(array == NULL)) {
        return NULL;
    }

    return ((header_t *)array) - 1;
}

PURE READ_ONLY(1) static inline size_t array_len(void *array) {
    header_t *header = get_header(array);

    if (UNLIKELY(header == NULL)) {
        return 0;
    }

    return header->length;
}

WARN_UNUSED int expand_if_needed(void **array);

#define array_append(__array, __value) ({ \
    /* unless we needed extra space and failed to allocate, append the item */ \
    int ret = expand_if_needed((void **)&__array); \
    if (ret == 0) { \
        header_t *header = get_header(__array); \
        if (header == NULL) { \
            ret = -ENOMEM; \
        } else { \
            __array[header->length++] = __value; \
        } \
    } \
    \
    ret; \
})

static inline void array_pop(void *array, size_t n_items) {
    header_t *header = get_header(array);
    header->length -= n_items;
}
