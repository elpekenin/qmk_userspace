// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/graphics.h"

#include <quantum/color.h>
#include <quantum/quantum.h>
#include <quantum/wpm.h>

#include "elpekenin/build_info.h"
#include "elpekenin/keylog.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/memory.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/string.h"
#include "elpekenin/time.h"

// compat: function must exist
#if defined(COMMUNITY_MODULE_ALLOCATOR_ENABLE)
#    include "elpekenin/allocator.h"
#else
#    define get_used_heap() 0
#endif

#if defined(COMMUNITY_MODULE_GLITCH_TEXT_ENABLE)
#    include "elpekenin/glitch_text.h"
#else
#    error Must enable 'elpekenin/glitch_text'
#endif

#if defined(COMMUNITY_MODULE_RNG_ENABLE)
#    include "elpekenin/rng.h"
#else
#    error Must enable 'elpekenin/rng'
#endif

// logging task
static deferred_token     logging_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t logging_args  = {0};

static uint32_t logging_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return MILLISECONDS(100);
}

void set_logging_device(painter_device_t device) {
    logging_args.device = device;
    cancel_deferred_exec(logging_token);
    logging_token = defer_exec(MILLISECONDS(10), logging_task_callback, &logging_args);
}

// uptime task
static deferred_token     uptime_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t uptime_args  = {0};

static uint32_t uptime_task_callback(uint32_t trigger_time, void *cb_arg) {
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

void set_uptime_device(painter_device_t device) {
    uptime_args.device = device;
    cancel_deferred_exec(uptime_token);
    uptime_token = defer_exec(MILLISECONDS(10), uptime_task_callback, &uptime_args);
}

// layer task
static bool               layer_running = false;
static deferred_token     layer_token   = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t layer_args    = {0};

static void draw_layer(const char *text, bool last_frame) {
    // TODO: do not hardcode access to global
    qp_callback_args_t *args = &layer_args;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // sat = 0 => white regardless of hue
    uint8_t hue = rng_min_max(0, UINT8_MAX); // random color
    uint8_t sat = UINT8_MAX;                 // state->running ? 255 : 0;

    qp_drawtext_recolor(args->device, args->x, args->y, args->font, text, hue, sat, UINT8_MAX, HSV_BLACK);

    if (last_frame) {
        layer_running = false;
    }
}

static uint32_t layer_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    static uint8_t last_layer = UINT8_MAX;

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    if (args->device == NULL || args->font == NULL || last_layer == layer || layer_running) {
        return MILLISECONDS(100);
    }

    last_layer    = layer;
    layer_running = true;

    // start the animation
    glitch_text_start(get_layer_name(layer), draw_layer);

    return MILLISECONDS(100);
}

void set_layer_device(painter_device_t device) {
    layer_args.device = device;
    cancel_deferred_exec(layer_token);
    layer_token = defer_exec(MILLISECONDS(10), layer_task_callback, &layer_args);
}

static bool               heap_stats_running = false;
static deferred_token     heap_stats_token   = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t heap_stats_args    = {0};

static void draw_heap(const char *text, __unused bool last_frame) {
    // TODO
    qp_callback_args_t *args = &heap_stats_args;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // heading space to align the ":" with flash
    int16_t width = qp_drawtext(args->device, args->x, args->y, args->font, " Heap: ");

    qp_drawtext(args->device, args->x + width, args->y, args->font, text);
}

static uint32_t heap_stats_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(COMMUNITY_MODULE_ALLOCATOR_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    static size_t last_used = 0;
    const size_t  used_heap = get_used_heap();

    if (args->device == NULL || args->font == NULL || last_used == used_heap || heap_stats_running) {
        return SECONDS(1);
    }

    last_used          = used_heap;
    heap_stats_running = true;

    // on first go, draw the consumed flash
    string_t str = str_new(100);

    str_append(&str, "Flash: ");
    pretty_bytes(&str, get_used_flash());
    str_append(&str, "/");
    pretty_bytes(&str, get_flash_size());
    qp_drawtext(heap_stats_args.device, heap_stats_args.x, heap_stats_args.y - heap_stats_args.font->line_height + 2, heap_stats_args.font, str.ptr);

    // reset buffer
    str_reset(&str);

    pretty_bytes(&str, used_heap);
    str_append(&str, "/");
    pretty_bytes(&str, get_heap_size());

    glitch_text_start(str.ptr, draw_heap);

    return SECONDS(1);
}

void set_heap_stats_device(painter_device_t device) {
    heap_stats_args.device = device;
    cancel_deferred_exec(heap_stats_token);
    heap_stats_token = defer_exec(MILLISECONDS(10), heap_stats_task_callback, &heap_stats_args);
}

static deferred_token     keylog_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t keylog_args  = {0};

static uint32_t keylog_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !is_keylog_dirty()) {
        return MILLISECONDS(10);
    }

    const char *keylog = get_keylog();
    uint16_t    width  = qp_get_width(args->device);
    uint16_t    height = qp_get_height(args->device);

    int16_t textwidth = qp_textwidth(args->font, keylog);
    qp_rect(args->device, 0, height - args->font->line_height, width - textwidth, height, HSV_BLACK, true);

    // default to white, change it based on WPM (if enabled)
    hsv_t color = {HSV_WHITE};
    if (IS_DEFINED(WPM_ENABLE)) {
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

    qp_drawtext_recolor(args->device, width - textwidth, height - args->font->line_height, args->font, keylog, color.h, color.s, color.v, HSV_BLACK);

    return MILLISECONDS(10);
}

void set_keylog_device(painter_device_t device) {
    keylog_args.device = device;
    cancel_deferred_exec(keylog_token);
    keylog_token = defer_exec(MILLISECONDS(10), keylog_task_callback, &keylog_args);
}

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

    if (IS_DEFINED(COMMUNITY_MODULE_ALLOCATOR_ENABLE)) {
        heap_stats_args.font = font;
        heap_stats_args.x    = 50;
        heap_stats_args.y    = y;

        y += spacing;
    }

    layer_args.font = font;
    layer_args.x    = 70;
    layer_args.y    = y;

    if (IS_DEFINED(KEYLOG_ENABLE)) {
        keylog_args.font = font;
    }
}
