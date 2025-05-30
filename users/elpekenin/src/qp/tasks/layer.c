// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/layer.h"

#include "elpekenin/layers.h"

#if CM_ENABLED(GLITCH_TEXT)
#    include "elpekenin/glitch_text.h"
#else
#    error Must enable 'elpekenin/glitch_text'
#endif

#if CM_ENABLED(RNG)
#    include "elpekenin/rng.h"
#else
#    error Must enable 'elpekenin/rng'
#endif

static bool               running = false;
static qp_callback_args_t args    = {0};

static void draw_layer(const char *text, bool last_frame) {
    // FIXME:
    qp_callback_args_t *arg = &args;

    if (arg->device == NULL || arg->font == NULL) {
        return;
    }

    // sat = 0 => white regardless of hue
    const uint8_t hue = rng_min_max(0, UINT8_MAX);
    const uint8_t sat = last_frame ? 0 : UINT8_MAX;

    qp_drawtext_recolor(arg->device, arg->x, arg->y, arg->font, text, hue, sat, UINT8_MAX, HSV_BLACK);

    if (last_frame) {
        running = false;
    }
}

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!CM_ENABLED(GLITCH_TEXT)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    static uint8_t last_layer = UINT8_MAX;

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    if (args->device == NULL || args->font == NULL || last_layer == layer || running) {
        return MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL);
    }

    last_layer = layer;
    running    = true;

    // start the animation
    const glitch_text_config_t config = {
        .str      = get_layer_name(layer),
        .callback = draw_layer,
        .delay    = 30,
    };
    glitch_text_start(&config);

    return MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL);
}

qp_callback_args_t *get_layer_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}
