// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/autoconf_rt.h"

#include <quantum/util.h>
#include <stdbool.h>

#define ADD_ONE(_) +1
#define NUM_AUTOCONF (0 FOREACH_AUTOCONF(ADD_ONE))

static struct {
    bool               init;
    autoconf_setting_t settings[NUM_AUTOCONF];
} autoconf = {0};

static autoconf_value_t initialize_int(size_t value) {
    return (autoconf_value_t){
        .type    = AUTOCONF_INT,
        .integer = value,
    };
}

static autoconf_value_t initialize_str(const char *value) {
    return (autoconf_value_t){
        .type   = AUTOCONF_STRING,
        .string = value,
    };
}

#define INITIALIZE_VALUE(val) _Generic((val), int: initialize_int, char *: initialize_str)(val)

#define AUTOCONF_ENTRY(setting)                        \
    autoconf.settings[index++] = (autoconf_setting_t){ \
        .name  = #setting,                             \
        .value = INITIALIZE_VALUE(setting),            \
    };

static void autoconf_settings_init(void) {
    size_t index = 0;
    FOREACH_AUTOCONF(AUTOCONF_ENTRY)
}

const autoconf_setting_t *get_autoconf_settings(void) {
    if (!autoconf.init) {
        autoconf.init = true;
        autoconf_settings_init();
    }

    return autoconf.settings;
}

size_t autoconf_settings_count(void) {
    return NUM_AUTOCONF;
}
