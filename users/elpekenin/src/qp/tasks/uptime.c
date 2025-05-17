// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/uptime.h"

static qp_callback_args_t args = {0};

static uint32_t callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return SECONDS(1);
    }

    div_t result    = div((int)trigger_time, DAYS(1));
    result          = div(result.rem, HOURS(1));
    uint8_t hours   = result.quot;
    result          = div(result.rem, MINUTES(1));
    uint8_t minutes = result.quot;
    uint8_t seconds = result.rem / SECONDS(1);

    char uptime_str[16];
    snprintf(uptime_str, sizeof(uptime_str), "Up|%02dh%02dm%02ds", hours, minutes, seconds);

    qp_drawtext(args->device, args->x, args->y, args->font, uptime_str);

    return SECONDS(1);
}

qp_callback_args_t *get_uptime_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}
