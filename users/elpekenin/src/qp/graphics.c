// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/graphics.h"

#include <errno.h>
#include <quantum/color.h>
#include <quantum/quantum.h>

#include "elpekenin/build_info.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/memory.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/qp/glitch_text.h"
#include "elpekenin/string.h"
#include "elpekenin/time.h"

#if defined(COMMUNITY_MODULE_ALLOCATOR_ENABLE)
#    include "elpekenin/allocator.h"
#endif

#if defined(COMMUNITY_MODULE_RNG_ENABLE)
#    include "elpekenin/rng.h"
#else
#    error "This code depends on 'elpekenin/rng' to work"
#endif

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif

// logging task
static deferred_token     logging_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t logging_args  = {0};

static uint32_t logging_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return 100;
}

void set_logging_device(painter_device_t device) {
    logging_args.device = device;
    cancel_deferred_exec(logging_token);
    logging_token = defer_exec(10, logging_task_callback, &logging_args);
}

// uptime task
static deferred_token     uptime_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t uptime_args  = {0};

static uint32_t uptime_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return 1000;
    }

    div_t result = div(trigger_time, MS_IN_A_DAY);
    // uint8_t days = result.quot;

    result          = div(result.rem, MS_IN_AN_HOUR);
    uint8_t hours   = result.quot;
    result          = div(result.rem, MS_IN_A_MIN);
    uint8_t minutes = result.quot;
    uint8_t seconds = result.rem / MS_IN_A_SEC;

    char uptime_str[16];
    snprintf(uptime_str, sizeof(uptime_str), "Up|%02dh%02dm%02ds", hours, minutes, seconds);

    qp_drawtext(args->device, args->x, args->y, args->font, uptime_str);

    return 1000;
}

void set_uptime_device(painter_device_t device) {
    uptime_args.device = device;
    cancel_deferred_exec(uptime_token);
    uptime_token = defer_exec(10, uptime_task_callback, &uptime_args);
}

// layer task
static deferred_token      layer_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t  layer_args  = {0};
static glitch_text_state_t layer_extra = {0};

static void draw_layer(void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // sat = 0 => white regardless of hue
    uint8_t hue = rng_min_max(0, 255); // random color
    uint8_t sat = state->running ? 255 : 0;

    qp_drawtext_recolor(args->device, args->x, args->y, args->font, state->curr, hue, sat, 255, HSV_BLACK);
}

static uint32_t layer_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    static uint8_t last_layer = UINT8_MAX;

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    if (args->device == NULL || args->font == NULL || last_layer == layer || state->running) {
        return 100;
    }

    last_layer     = layer;
    state->running = true;

    // start the animation
    strlcpy(state->dest, get_layer_name(layer), sizeof(state->dest));
    state->mask  = 0;
    state->state = FILLING;
    defer_exec(10, glitch_text_callback, args);

    return 100;
}

void set_layer_device(painter_device_t device) {
    layer_args.device = device;
    cancel_deferred_exec(layer_token);
    layer_token = defer_exec(10, layer_task_callback, &layer_args);
}

#if defined(COMMUNITY_MODULE_ALLOCATOR_ENABLE)
static deferred_token      heap_stats_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t  heap_stats_args  = {0};
static glitch_text_state_t heap_stats_extra = {0};

static void draw_heap(void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // heading space to align the ":" with flash
    int16_t width = qp_drawtext(args->device, args->x, args->y, args->font, " Heap: ");

    qp_drawtext(args->device, args->x + width, args->y, args->font, state->curr);
}

static uint32_t heap_stats_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    static size_t last_used = 0;
    const size_t  used_heap = get_used_heap();

    if (args->device == NULL || args->font == NULL || last_used == used_heap || state->running) {
        return 1000;
    }
    last_used = used_heap;

    // on first go, draw the consumed flash
    string_t str = str_new(100);

    str_append(&str, "Flash: ");
    pretty_bytes(&str, get_used_flash());
    str_append(&str, "/");
    pretty_bytes(&str, get_flash_size());
    qp_drawtext(heap_stats_args.device, heap_stats_args.x, heap_stats_args.y - heap_stats_args.font->line_height + 2, heap_stats_args.font, str_get(str));

    // reset buffer
    str_reset(&str);

    pretty_bytes(&str, used_heap);
    str_append(&str, "/");
    pretty_bytes(&str, get_heap_size());

    strlcpy(state->dest, str_get(str), sizeof(state->dest));
    state->running = true;
    state->mask    = 0;
    state->state   = FILLING;
    defer_exec(10, glitch_text_callback, args);

    return 1000;
}

void set_heap_stats_device(painter_device_t device) {
    heap_stats_args.device = device;
    cancel_deferred_exec(heap_stats_token);
    heap_stats_token = defer_exec(10, heap_stats_task_callback, &heap_stats_args);
}
#endif

#if defined(KEYLOG_ENABLE)
static deferred_token     keylog_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t keylog_args  = {0};

static uint32_t keylog_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !is_keylog_dirty()) {
        return 10;
    }

    const char *keylog = get_keylog();
    uint16_t    width  = qp_get_width(args->device);
    uint16_t    height = qp_get_height(args->device);

    int16_t textwidth = qp_textwidth(args->font, keylog);
    qp_rect(args->device, 0, height - args->font->line_height, width - textwidth, height, HSV_BLACK, true);

    // default to white, change it based on WPM (if enabled)
    hsv_t color = {HSV_WHITE};

#    if defined(WPM_ENABLE)
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
#    endif

    qp_drawtext_recolor(args->device, width - textwidth, height - args->font->line_height, args->font, keylog, color.h, color.s, color.v, HSV_BLACK);

    return 10;
}

void set_keylog_device(painter_device_t device) {
    keylog_args.device = device;
    cancel_deferred_exec(keylog_token);
    keylog_token = defer_exec(10, keylog_task_callback, &keylog_args);
}
#endif

// *** Build info ***

void draw_commit(painter_device_t device) {
    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    painter_driver_t *driver     = (painter_driver_t *)device;
    uint16_t          width      = driver->panel_width;
    uint16_t          height     = driver->panel_height;
    int16_t           hash_width = qp_textwidth(font, get_build_info().commit);

    uint16_t real_width  = width;
    uint16_t real_height = height;

    if (driver->rotation == QP_ROTATION_90 || driver->rotation == QP_ROTATION_270) {
        real_width  = height;
        real_height = width;
    }

    uint16_t x = real_width - hash_width;
    uint16_t y = real_height - font->line_height;

    qp_drawtext_recolor(device, x, y, font, get_build_info().commit, HSV_RED, HSV_WHITE);
}

void qp_tasks_init(void) {
    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    // positions are hard-coded for ILI9341 on access
    uint16_t y       = 55;
    size_t   spacing = font->line_height + 2;

    logging_args.font                   = font;
    logging_args.x                      = 160;
    logging_args.y                      = 100;
    logging_args.scrolling_args.n_chars = 18;
    logging_args.scrolling_args.delay   = 500;

    uptime_args.font = font;
    uptime_args.x    = 50;
    uptime_args.y    = y;

    y += spacing;

#if defined(COMMUNITY_MODULE_ALLOCATOR_ENABLE)
    heap_stats_args.font      = font;
    heap_stats_args.x         = 50;
    heap_stats_args.y         = y;
    heap_stats_extra.callback = draw_heap;
    heap_stats_args.extra     = &heap_stats_extra;

    y += spacing;
#endif

    layer_args.font      = font;
    layer_args.x         = 70;
    layer_args.y         = y;
    layer_extra.callback = draw_layer;
    layer_args.extra     = &layer_extra;

#if defined(KEYLOG_ENABLE)
    keylog_args.font = font;
#endif
}
