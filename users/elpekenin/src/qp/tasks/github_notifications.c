// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/github_notifications.h"

#include "elpekenin/qp/assets.h"

static qp_callback_args_t args = {0};

static uint8_t notifications = 0;

static bool redraw = false;

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(XAP_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !redraw) {
        return MILLISECONDS(100);
    }

    const painter_image_handle_t logo = qp_get_image_by_name("github");
    if (logo == NULL) {
        return 0;
    }

    qp_drawimage(args->device, args->x, args->y, logo);
    redraw = false;

    const char *text = get_u8_str(notifications, ' ');
    qp_drawtext(args->device, args->x, args->y, args->font, text);

    return MILLISECONDS(200);
}

qp_callback_args_t *get_github_notifications_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}

void set_github_notifications_count(uint8_t count) {
    notifications = count;
    redraw        = true;
}
