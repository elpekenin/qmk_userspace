// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/layer.h"

#include <quantum/compiler_support.h>

#include "elpekenin/layers.h"

STATIC_ASSERT(CM_ENABLED(GLITCH_TEXT), "Must enable 'elpekenin/glitch_text'");
STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
STATIC_ASSERT(CM_ENABLED(RNG), "Must enable 'elpekenin/rng'");

#include "elpekenin/glitch_text.h"
#include "elpekenin/logging.h"
#include "elpekenin/rng.h"

static struct {
    bool               running;
    qp_callback_args_t args;
    uint8_t            last;
} layer = {
    .running = false,
    .args    = {0},
    .last    = UINT8_MAX,
};

static void draw_layer(const char *text, bool last_frame) {
    // FIXME:
    qp_callback_args_t *arg = &layer.args;

    if (arg->device == NULL || arg->font == NULL) {
        return;
    }

    // sat = 0 => white regardless of hue
    const uint8_t hue = rng_min_max(0, UINT8_MAX);
    const uint8_t sat = last_frame ? 0 : UINT8_MAX;

    qp_drawtext_recolor(arg->device, arg->x, arg->y, arg->font, text, hue, sat, UINT8_MAX, HSV_BLACK);

    if (last_frame) {
        layer.running = false;
    }
}

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!CM_ENABLED(GLITCH_TEXT)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    const uint8_t current_layer = get_highest_layer(layer_state | default_layer_state);

    if (args->device == NULL || args->font == NULL || layer.last == current_layer || layer.running) {
        return MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL);
    }

    // start the animation
    const glitch_text_config_t config = {
        .callback = draw_layer,
        .delay    = 30,
#if GLITCH_TEXT_USE_ALLOCATOR
        .allocator = c_runtime_allocator,
#endif
    };

    const int ret = glitch_text_start(&config, get_layer_name(current_layer));
    if (ret < 0) {
        logging(LOG_ERROR, "%s: Could not initialize glitch text", __func__);
        return MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL);
    }

    layer.last    = current_layer;
    layer.running = true;

    return MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL);
}

qp_callback_args_t *get_layer_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &layer.args);

    return &layer.args;
}
