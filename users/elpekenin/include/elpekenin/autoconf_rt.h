
// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Inspect autoconf settings at run-time
 */

// -- barrier --

#pragma once

#include <stddef.h>

typedef enum {
    AUTOCONF_INT,
    AUTOCONF_STRING,
} autoconf_type_t;

typedef struct {
    autoconf_type_t type;
    union {
        size_t      integer;
        const char *string;
    };
} autoconf_value_t;

typedef struct {
    const char      *name;
    autoconf_value_t value;
} autoconf_setting_t;

size_t autoconf_settings_count(void);

const autoconf_setting_t *get_autoconf_settings(void);
