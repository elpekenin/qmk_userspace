// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/flash.h"

#include "elpekenin/memory.h"

STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");
#include "elpekenin/string.h"

bool flash_init(ui_node_t *self) {
    flash_args_t *const args = self->args;

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return false;
    }

    const uint8_t line_height = font->line_height;
    qp_close_font(font);

    if (line_height > self->size.y) {
        return false;
    }

    args->last = ~0;
    return true;
}

void flash_render(const ui_node_t *self, painter_device_t display) {
    flash_args_t *const args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_FLASH_REDRAW_INTERVAL))) {
        return;
    }

    const size_t flash = get_used_flash();
    if (args->last == flash) {
        return;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return;
    }

    string_t str = str_new(30);

    str_append(&str, "Flash: ");
    pretty_bytes(&str, flash);
    str_append(&str, "/");
    pretty_bytes(&str, get_flash_size());

    const uint16_t width = qp_textwidth(font, str.ptr);
    if (width == 0 || width > self->size.x) {
        goto exit;
    }

    qp_drawtext(display, self->start.x, self->start.y, font, str.ptr);

    args->last = flash;

exit:
    qp_close_font(font);
}
