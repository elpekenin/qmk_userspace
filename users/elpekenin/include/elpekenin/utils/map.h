// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"
#include "elpekenin/utils/shortcuts.h"

// a map is just a convenience wrapper for two dynamic arrays

#define new_map(__type, __name) \
    struct { \
        const char **keys; \
        __type      *values; \
    } __name

#define map_init(__map, __size, __allocator) \
    do { \
        __map.keys   = new_array(         const char *, __size, __allocator); \
        __map.values = new_array(typeof(*__map.values), __size, __allocator); \
    } while(0)

// can not overwrite
// does not handle errors if array_append fails
// which should be very unlikely (no space left to allocate)
#define map_set(__map, __key, __value) \
    do { \
        /* if key already used, do nothing */ \
        int ret; \
        WITHOUT_LOGGING( \
            MAP, \
            map_get(__map, __key, ret); \
        ); \
        if (ret == -ENOTFOUND) { \
            /* dont bother with value if key fails */ \
            if (array_append(__map.keys, __key) == 0) { \
                /* if pushing value fails, pop key */ \
                if (array_append(__map.values, __value) != 0) { \
                    array_pop(__map.keys, 1); \
                } \
            } \
        } \
    } while (0)

#define map_get(__map, __key, __ret) ({ \
    __ret = -ENOTFOUND; \
    size_t i; \
    for (i = 0; i < array_len(__map.keys); ++i) { \
        if (__map.keys[i] && strcmp(__map.keys[i], __key) == 0) { \
            __ret = 0; \
            _ = logging(MAP, LOG_DEBUG, "Read '%s'", __key); \
            break; /* without this, we get one off (next ++) */ \
        } \
    } \
    \
    if (__ret < 0) { \
        _ = logging(MAP, LOG_ERROR, "Key '%s' not found", __key); \
    } \
    \
    __map.values[i]; \
})
