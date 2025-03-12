// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/sections.h"

#ifndef LOG_N_LINES
#    define LOG_N_LINES 9
#endif

#ifndef LOG_N_CHARS
#    define LOG_N_CHARS 70
#endif

static char           qp_log[LOG_N_LINES][LOG_N_CHARS + 1];
static uint8_t        qp_log_current_col;
static char          *qp_log_pointers[LOG_N_LINES];
static deferred_token qp_log_tokens[LOG_N_LINES];
static bool           qp_log_redraw;
static log_level_t    qp_log_levels[LOG_N_LINES];

static void qp_log_init(void) {
    memset(qp_log, 0, sizeof(qp_log));
    for (int8_t i = 0; i < LOG_N_LINES; ++i) {
        qp_log_pointers[i] = qp_log[i];
        qp_log_tokens[i]   = INVALID_DEFERRED_TOKEN;
    }
    qp_log_redraw = false;
}
PEKE_PRE_INIT(qp_log_init, INIT_QP_LOG);

static int8_t sendchar_qp_hook(uint8_t c) {
    if (c == '\n') {
        // Add null pointer to current line
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col] = 0;

        // Move everything 1 line upwards
        char *temp = qp_log_pointers[0];
        for (int8_t i = 0; i < LOG_N_LINES - 1; ++i) {
            qp_log_pointers[i] = qp_log_pointers[i + 1];
            qp_log_levels[i]   = qp_log_levels[i + 1];
        }
        qp_log_pointers[LOG_N_LINES - 1] = temp;
        qp_log_levels[LOG_N_LINES - 1]   = LOG_NONE;

        // Reset stuff
        qp_log_current_col                                   = 0;
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col] = '\0';
        qp_log_redraw                                        = true;
    } else if (qp_log_current_col >= LOG_N_CHARS) {
        return 0;
    } else {
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col++] = c;
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col]   = '\0';
        qp_log_levels[LOG_N_LINES - 1]                         = get_message_level();
        qp_log_redraw                                          = true;
    }

    return 0;
}
PEKE_SENDCHAR(sendchar_qp_hook);

void qp_log_clear(void) {
    for (int8_t i = 0; i < LOG_N_LINES; ++i) {
        sendchar_qp_hook('\n');
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
#if defined(QUANTUM_PAINTER_DEBUG)
    return;
#endif

    if (!qp_log_redraw || args->device == NULL) {
        return;
    }

    qp_log_redraw = false;

    // Clear space
    qp_rect(args->device, args->x, args->y, qp_get_width(args->device), args->y + LOG_N_LINES * args->font->line_height, HSV_BLACK, true);

    uint16_t y = args->y;
    for (int8_t i = 0; i < LOG_N_LINES; ++i) {
        int16_t textwidth = qp_textwidth(args->font, (const char *)qp_log_pointers[i]);

        bool text_fits = textwidth < (qp_get_width(args->device) - args->x);

        stop_scrolling_text(qp_log_tokens[i]);
        qp_log_tokens[i] = INVALID_DEFERRED_TOKEN;

        y += args->font->line_height;

        hsv_t bg = {HSV_BLACK};
        hsv_t fg = log_colors[qp_log_levels[i]];

        if (text_fits) {
            qp_drawtext_recolor(args->device, args->x, y, args->font, (const char *)qp_log_pointers[i], fg.h, fg.s, fg.v, bg.h, bg.s, bg.v);
        } else {
            qp_log_tokens[i] = draw_scrolling_text_recolor(args->device, args->x, y, args->font, (const char *)qp_log_pointers[i], args->scrolling_args.n_chars, args->scrolling_args.delay, fg.h, fg.s, fg.v, bg.h, bg.s, bg.v);
        }
    }
}
