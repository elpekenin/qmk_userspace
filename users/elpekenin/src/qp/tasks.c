// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks.h"

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

#if defined(COMMUNITY_MODULE_QP_HELPERS_ENABLE)
#    include "qp_helpers.h"
#else
#    define qp_draw_graph(...) ((void)0)
#endif

// logging task
static qp_callback_args_t logging_args = {0};

static uint32_t logging_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(COMMUNITY_MODULE_LOGGING_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return MILLISECONDS(100);
}

// uptime task
static qp_callback_args_t uptime_args = {0};

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

// layer task
static bool               layer_running = false;
static qp_callback_args_t layer_args    = {0};

static void draw_layer(const char *text, bool last_frame) {
    // TODO: do not hardcode access to global
    qp_callback_args_t *args = &layer_args;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // sat = 0 => white regardless of hue
    const uint8_t hue = rng_min_max(0, UINT8_MAX);
    const uint8_t sat = last_frame ? 0 : UINT8_MAX;

    qp_drawtext_recolor(args->device, args->x, args->y, args->font, text, hue, sat, UINT8_MAX, HSV_BLACK);

    if (last_frame) {
        layer_running = false;
    }
}

static uint32_t layer_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(COMMUNITY_MODULE_GLITCH_TEXT_ENABLE)) {
        return 0;
    }

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

// heap task
static bool               heap_stats_running = false;
static qp_callback_args_t heap_stats_args    = {0};

static void draw_heap(const char *text, __unused bool last_frame) {
    // FIXME:
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

// keylog task
static qp_callback_args_t keylog_args = {0};

static uint32_t keylog_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(KEYLOG_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !is_keylog_dirty()) {
        return MILLISECONDS(10);
    }

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

    const char *keylog = get_keylog();
    qp_drawtext_recolor(args->device, args->x, args->y, args->font, keylog, color.h, color.s, color.v, HSV_BLACK);

    return MILLISECONDS(10);
}

// CPU/RAM task
#ifndef COMPUTER_STATS_ARRAY_SIZE
#    define COMPUTER_STATS_ARRAY_SIZE 10
#endif

static qp_callback_args_t computer_stats_args = {0};

static struct {
    uint8_t cpu[COMPUTER_STATS_ARRAY_SIZE];
    uint8_t ram[COMPUTER_STATS_ARRAY_SIZE];
} computer_data = {0};

static bool computer_needs_redraw = false;

static uint32_t computer_task_callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(XAP_ENABLE)) {
        return 0;
    }

    if (!IS_DEFINED(COMMUNITY_MODULE_QP_HELPERS_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !computer_needs_redraw) {
        return MILLISECONDS(100);
    }

    const uint16_t max_value  = 100;
    const uint16_t graph_size = (uintptr_t)args->extra;

    const hsv_t axis_color = {HSV_WHITE};
    const hsv_t bg_color   = {HSV_BLACK};

    const graph_line_t graphs[] = {
        {
            .line_data  = computer_data.cpu,
            .line_color = {HSV_BLUE},
            .mode       = LINE,
        },
        {
            .line_data  = computer_data.ram,
            .line_color = {HSV_YELLOW},
            .mode       = LINE,
        },
    };

    qp_draw_graph(args->device, args->x, args->y, graph_size, graph_size, axis_color, bg_color, graphs, ARRAY_SIZE(graphs), COMPUTER_STATS_ARRAY_SIZE, max_value);
    computer_needs_redraw = false;

    return MILLISECONDS(200);
}

//
// public API
//
//
// NOTE: these functions always return `NULL` after being called once
//       this is to prevent overwriting the configuration when (if) task is configured
//

qp_callback_args_t *get_logging_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), logging_task_callback, &logging_args);

    return &logging_args;
}

qp_callback_args_t *get_uptime_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), uptime_task_callback, &uptime_args);

    return &uptime_args;
}

qp_callback_args_t *get_layer_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), layer_task_callback, &layer_args);

    return &layer_args;
}

qp_callback_args_t *get_heap_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), heap_stats_task_callback, &heap_stats_args);

    return &heap_stats_args;
}

qp_callback_args_t *get_keylog_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), keylog_task_callback, &keylog_args);

    return &keylog_args;
}

qp_callback_args_t *get_computer_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), computer_task_callback, &computer_stats_args);

    return &computer_stats_args;
}

//

void draw_commit(painter_device_t device) {
    painter_font_handle_t font = qp_get_font_by_name("fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    uint16_t width  = qp_get_width(device);
    uint16_t height = qp_get_height(device);

    const char *commit     = get_build_info().commit;
    int16_t     hash_width = qp_textwidth(font, commit);

    uint16_t x = width - hash_width;
    uint16_t y = height - font->line_height;

    qp_drawtext_recolor(device, x, y, font, commit, HSV_RED, HSV_WHITE);
}

void push_computer_stats(uint8_t cpu, uint8_t ram) {
    memmove(computer_data.cpu + 1, computer_data.cpu, COMPUTER_STATS_ARRAY_SIZE - 1);
    memmove(computer_data.ram + 1, computer_data.ram, COMPUTER_STATS_ARRAY_SIZE - 1);

    computer_data.cpu[0] = cpu;
    computer_data.ram[0] = ram;

    computer_needs_redraw = true;
}
