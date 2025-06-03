// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/github_notifications.h"

#include "elpekenin/qp/assets.h"
#include "elpekenin/xap.h"

static struct {
    qp_callback_args_t args;
    uint8_t            count;
    bool               redraw;
    bool               clear;
} github_notifications = {0};

static const char *as_text(uint8_t value) {
    static char text[2] = {0, 0};

    switch (value) {
        case 0 ... 9:
            text[0] = value + '0';
            break;

        default:
            text[0] = '!';
            break;
    }

    return text;
}

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(XAP)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return MILLISECONDS(QP_TASK_GITHUB_NOTIFICATIONS_REDRAW_INTERVAL);
    }

    const painter_image_handle_t logo = qp_get_image_by_name("github");
    if (logo == NULL) {
        return 0;
    }

    if (xap_last_activity_elapsed() > MILLISECONDS(QP_TASK_GITHUB_NOTIFICATIONS_TIMEOUT)) {
        if (github_notifications.clear) {
            // clear previous drawings
            qp_rect(args->device, args->x, args->y, args->x + logo->width, args->y + logo->height, HSV_BLACK, true);
            qp_drawtext(args->device, args->x, args->y, args->font, " ");
        }

        github_notifications.clear = false;
    } else {
        github_notifications.clear = true;

        if (github_notifications.redraw) {
            const hsv_t color = (github_notifications.count == 0) ? (hsv_t){HSV_GREEN} : (hsv_t){HSV_RED};
            qp_drawimage(args->device, args->x, args->y, logo);
            qp_drawtext_recolor(args->device, args->x, args->y, args->font, as_text(github_notifications.count), color.h, color.s, color.v, HSV_BLACK);
        }

        github_notifications.redraw = false;
    }

    return MILLISECONDS(QP_TASK_GITHUB_NOTIFICATIONS_REDRAW_INTERVAL);
}

qp_callback_args_t *get_github_notifications_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &github_notifications.args);

    return &github_notifications.args;
}

void set_github_notifications_count(uint8_t count) {
    github_notifications.count  = count;
    github_notifications.redraw = true;
}
