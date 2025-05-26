// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/heap_stats.h"

#include "elpekenin/memory.h"

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

#if defined(COMMUNITY_MODULE_STRING_ENABLE)
#    include "elpekenin/string.h"
#else
#    error Must enable 'elpekenin/string'
#endif

static bool               running = false;
static qp_callback_args_t args    = {0};

static void draw_heap(const char *text, __unused bool last_frame) {
    // FIXME:
    qp_callback_args_t *arg = &args;

    if (arg->device == NULL || arg->font == NULL) {
        return;
    }

    // heading space to align the ":" with flash
    int16_t width = qp_drawtext(arg->device, arg->x, arg->y, arg->font, " Heap: ");

    qp_drawtext(arg->device, arg->x + width, arg->y, arg->font, text);
}

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(COMMUNITY_MODULE_ALLOCATOR)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    static size_t last_used = 0;
    const size_t  used_heap = get_used_heap();

    if (args->device == NULL || args->font == NULL || last_used == used_heap || running) {
        return SECONDS(1);
    }

    last_used = used_heap;
    running   = true;

    // on first go, draw the consumed flash
    string_t str = str_new(100);

    str_append(&str, "Flash: ");
    pretty_bytes(&str, get_used_flash());
    str_append(&str, "/");
    pretty_bytes(&str, get_flash_size());
    qp_drawtext(args->device, args->x, args->y - args->font->line_height + 2, args->font, str.ptr);

    // reset buffer
    str_reset(&str);

    pretty_bytes(&str, used_heap);
    str_append(&str, "/");
    pretty_bytes(&str, get_heap_size());

    glitch_text_start(str.ptr, draw_heap);

    return SECONDS(1);
}

qp_callback_args_t *get_heap_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}
