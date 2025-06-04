// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging/backends/qp.h"

#include <errno.h>
#include <quantum/color.h>
#include <quantum/compiler_support.h>
#include <quantum/quantum.h>

#include "elpekenin/qp/tasks/common.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
STATIC_ASSERT(CM_ENABLED(SCROLLING_TEXT), "Must enable 'elpekenin/scrolling_text'");

#include "elpekenin/logging.h"
#include "elpekenin/scrolling_text.h"

typedef struct {
    char          *ptr;
    log_level_t    level;
    deferred_token token;
} log_line_t;

static struct {
    char       buff[QP_LOG_N_LINES][QP_LOG_N_CHARS];
    size_t     col;
    bool       redraw;
    log_line_t lines[QP_LOG_N_LINES];
} qp_log = {0};

void sendchar_qp_init(void) {
    for (size_t i = 0; i < QP_LOG_N_LINES; ++i) {
        qp_log.lines[i].ptr = qp_log.buff[i];
    }
}

int8_t sendchar_qp(uint8_t chr) {
    log_line_t *const last_line = &qp_log.lines[QP_LOG_N_LINES - 1];

    // no-op, always appending a terminator upon writing a char
    if (chr == '\0') {
        return 0;
    }

    // newline, move everything 1 line upwards
    if (chr == '\n') {
        log_line_t overwritten = qp_log.lines[0];
        scrolling_text_stop(overwritten.token);

        memmove(qp_log.lines, qp_log.lines + 1, sizeof(log_line_t) * (QP_LOG_N_LINES - 1));

        qp_log.col = 0;
        *last_line = (log_line_t){
            .ptr   = overwritten.ptr,
            .level = LOG_NONE,
            .token = INVALID_DEFERRED_TOKEN,
        };
        *last_line->ptr = '\0';

        qp_log.redraw = true;
        return 0;
    }

    // exhausted buffer
    if (qp_log.col >= (QP_LOG_N_CHARS - 1)) {
        return -ENOMEM;
    }

    // regular buffering
    last_line->ptr[qp_log.col++] = chr;
    last_line->ptr[qp_log.col]   = '\0';

    last_line->level = get_current_message_level();

    qp_log.redraw = true;
    return 0;
}

void qp_log_clear(void) {
    for (size_t i = 0; i < QP_LOG_N_LINES; ++i) {
        sendchar_qp('\n');
    }
}

// clang-format off
static const hsv_t log_colors[] = {
    [LOG_NONE]  = {HSV_WHITE},
    [LOG_DEBUG] = {0, 0, 100},
    [LOG_INFO]  = {HSV_BLUE},
    [LOG_WARN]  = {HSV_ORANGE},
    [LOG_ERROR] = {HSV_RED},
};
// clang-format on
ASSERT_LEVELS(log_colors);

void qp_logging_backend_render(qp_callback_args_t *args) {
    if (IS_DEFINED(QUANTUM_PAINTER_DEBUG)) {
        return;
    }

    if (!qp_log.redraw || args->device == NULL || args->font == NULL) {
        return;
    }
    qp_log.redraw = false;

    const uint16_t font_height = args->font->line_height;

    // Clear space
    qp_rect(args->device, args->x, args->y, qp_get_width(args->device), args->y + (QP_LOG_N_LINES * font_height), HSV_BLACK, true);

    uint16_t y = args->y - font_height;
    for (size_t i = 0; i < QP_LOG_N_LINES; ++i) {
        log_line_t *const line = &qp_log.lines[i];

        scrolling_text_stop(line->token);
        line->token = INVALID_DEFERRED_TOKEN;

        y += font_height;

        const hsv_t fg = log_colors[line->level];
        const hsv_t bg = {HSV_BLACK};

        const bool text_fits = qp_textwidth(args->font, line->ptr) < (qp_get_width(args->device) - args->x);
        if (text_fits) {
            qp_drawtext_recolor(args->device, args->x, y, args->font, line->ptr, fg.h, fg.s, fg.v, bg.h, bg.s, bg.v);
            continue;
        }

        const scrolling_text_config_t config = {
            .device  = args->device,
            .x       = args->x,
            .y       = y,
            .font    = args->font,
            .n_chars = args->scrolling_args.n_chars,
            .delay   = args->scrolling_args.delay,
            .spaces  = 5,
            .fg      = fg,
            .bg      = bg,
#if SCROLLING_TEXT_USE_ALLOCATOR
            .allocator = c_runtime_allocator,
#endif
        };

        line->token = scrolling_text_start(&config, line->ptr);
    }
}
