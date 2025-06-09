// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging/backends/qp.h"

#include <errno.h>
#include <quantum/color.h>
#include <quantum/compiler_support.h>
#include <quantum/quantum.h>

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
#include "elpekenin/logging.h"

typedef struct {
    char       *ptr;
    log_level_t level;
} log_line_t;

static struct {
    char       buff[QP_LOG_N_LINES][QP_LOG_N_CHARS];
    size_t     col;
    uint32_t   last;
    log_line_t lines[QP_LOG_N_LINES];
} qp_log = {0};

//
// sendchar implementation (buffering)
//

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

        memmove(qp_log.lines, qp_log.lines + 1, sizeof(log_line_t) * (QP_LOG_N_LINES - 1));

        qp_log.col = 0;
        *last_line = (log_line_t){
            .ptr   = overwritten.ptr,
            .level = LOG_NONE,
        };
        *last_line->ptr = '\0';

        goto exit;
    }

    // exhausted buffer
    if (qp_log.col >= (QP_LOG_N_CHARS - 1)) {
        return -ENOMEM;
    }

    // regular buffering
    last_line->ptr[qp_log.col++] = chr;
    last_line->ptr[qp_log.col]   = '\0';

    last_line->level = get_current_message_level();

exit:
    qp_log.last = timer_read32();
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

//
// ui rendering
//
#if CM_ENABLED(UI)
#    include "elpekenin/ui/utils.h"

bool qp_logging_init(ui_node_t *self) {
    return ui_font_fits(self);
}

uint32_t qp_logging_render(const ui_node_t *self, painter_device_t display) {
    qp_logging_args_t *args = self->args;

    if (qp_log.last <= args->last) {
        goto exit;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        goto exit;
    }

    // clear
    qp_rect(display, self->start.x, self->start.y, self->start.x + self->size.x, self->start.y + self->size.y, HSV_BLACK, true);

    for (size_t i = 0; i < QP_LOG_N_LINES; ++i) {
        const uint16_t y = self->start.y + (i * font->line_height);

        // can't fit more lines
        if ((y + font->line_height) > self->size.y) {
            break;
        }

        log_line_t *const line = &qp_log.lines[i];

        const hsv_t fg = log_colors[line->level];
        const hsv_t bg = {HSV_BLACK};

        if (!ui_text_fits(self, font, line->ptr)) {
            continue;
        }

        qp_drawtext_recolor(display, self->start.x, y, font, line->ptr, fg.h, fg.s, fg.v, bg.h, bg.s, bg.v);
    }

    qp_close_font(font);

    args->last = timer_read32();

exit:
    return QP_LOGGING_UI_REDRAW_INTERVAL;
}
#endif
