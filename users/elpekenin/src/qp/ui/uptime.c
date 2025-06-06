// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/uptime.h"

STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");
#include "elpekenin/string.h"

bool uptime_init(ui_node_t *self) {
    uptime_args_t *const args = self->args;

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return false;
    }

    const uint8_t line_height = font->line_height;
    qp_close_font(font);

    return line_height <= self->size.y;
}

void uptime_render(const ui_node_t *self, painter_device_t display) {
    uptime_args_t *args = self->args;

    if (!task_should_draw(&args->timer, SECONDS(1))) {
        return;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return;
    }

    const div_t   days    = div(args->timer, DAYS(1));
    const div_t   hours   = div(days.rem, HOURS(1));
    const div_t   minutes = div(hours.rem, MINUTES(1));
    const uint8_t seconds = minutes.rem / SECONDS(1);

    string_t uptime = str_new(16);
    str_printf(&uptime, "Up|%02dh%02dm%02ds", hours.quot, minutes.quot, seconds);

    // text does not fit in bounds (width=0 means error)
    const int16_t width = qp_textwidth(font, uptime.ptr);
    if (width == 0 || width > self->size.x) {
        goto exit;
    }

    qp_drawtext(display, self->start.x, self->start.y, font, uptime.ptr);

exit:
    qp_close_font(font);
}
