// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/keylog.h"

#include "elpekenin/keylog.h"

#if 0
static qp_callback_args_t args = {0};

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(KEYLOG)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !is_keylog_dirty()) {
        return MILLISECONDS(QP_TASK_KEYLOG_REDRAW_INTERVAL);
    }

    // default to white, change it based on WPM (if enabled)
    hsv_t color = {HSV_WHITE};
    if (IS_ENABLED(WPM)) {
        uint8_t wpm = get_current_wpm();

        if (wpm > 10) {
            color = (hsv_t){HSV_RED};
        }
        if (wpm > 30) {
            color = (hsv_t){HSV_YELLOW};
        }
        if (wpm > 50) {
            color = (hsv_t){HSV_GREEN};
        }
    }

    const char *keylog = get_keylog();
    qp_drawtext_recolor(args->device, args->x, args->y, args->font, keylog, color.h, color.s, color.v, HSV_BLACK);

    return MILLISECONDS(QP_TASK_KEYLOG_REDRAW_INTERVAL);
}

qp_callback_args_t *get_keylog_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}
#endif
