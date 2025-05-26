// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/logging.h"

#include "elpekenin/logging/backends/qp.h"

static qp_callback_args_t args = {0};

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(COMMUNITY_MODULE_LOGGING)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw

    return MILLISECONDS(100);
}

qp_callback_args_t *get_logging_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}
