// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/heap_stats.h"

#include "elpekenin/memory.h"

// compat: function must exist
#if CM_ENABLED(ALLOCATOR)
#    include "elpekenin/allocator.h"
#else
#    define get_used_heap() 0
#endif

#if CM_ENABLED(GLITCH_TEXT)
#    include "elpekenin/glitch_text.h"
#else
#    error Must enable 'elpekenin/glitch_text'
#endif

#if CM_ENABLED(LOGGING)
#    include "elpekenin/logging.h"
#else
#    error Must enable 'elpekenin/logging'
#endif

#if CM_ENABLED(STRING)
#    include "elpekenin/string.h"
#else
#    error Must enable 'elpekenin/string'
#endif

static struct {
    qp_callback_args_t args;
    bool               running;
    size_t             last;
} heap_stats = {0};

static void draw_heap(const char *text, __unused bool last_frame) {
    // FIXME:
    qp_callback_args_t *arg = &heap_stats.args;

    if (arg->device == NULL || arg->font == NULL) {
        return;
    }

    string_t str = str_new(30);
    str_printf(&str, " Heap: %s", text); // heading space to align the ":" with "Flash:"
    qp_drawtext(arg->device, arg->x, arg->y + arg->font->line_height + 2, arg->font, str.ptr);

    if (last_frame) {
        heap_stats.running = false;
    }
}

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!CM_ENABLED(ALLOCATOR)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    const size_t used_heap = get_used_heap();

    if (args->device == NULL || args->font == NULL || heap_stats.last == used_heap || heap_stats.running) {
        return MILLISECONDS(QP_TASK_HEAP_STATS_REDRAW_INTERVAL);
    }

    // on first go, draw the consumed flash
    string_t str = str_new(50);

    str_append(&str, "Flash: ");
    pretty_bytes(&str, get_used_flash());
    str_append(&str, "/");
    pretty_bytes(&str, get_flash_size());
    qp_drawtext(args->device, args->x, args->y, args->font, str.ptr);

    // reset buffer
    str_reset(&str);

    pretty_bytes(&str, used_heap);
    str_append(&str, "/");
    pretty_bytes(&str, get_heap_size());

    const glitch_text_config_t config = {
        .callback = draw_heap,
        .delay    = 30,
    };

    const int ret = glitch_text_start(&config, str.ptr);
    if (ret < 0) {
        logging(LOG_ERROR, "%s: Could not initialize glitch text", __func__);
    }

    heap_stats.last    = used_heap;
    heap_stats.running = true;

    return MILLISECONDS(QP_TASK_HEAP_STATS_REDRAW_INTERVAL);
}

qp_callback_args_t *get_heap_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &heap_stats.args);

    return &heap_stats.args;
}
