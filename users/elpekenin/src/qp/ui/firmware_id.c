// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/firmware_id.h"

#include "elpekenin/split/transactions.h"

STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");
#include "elpekenin/string.h"

bool fw_id_init(ui_node_t *self) {
    fw_args_t *args = self->args;

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return false;
    }

    const uint8_t line_height = font->line_height;
    qp_close_font(font);

    return line_height <= self->size.y;
}

void fw_id_render(const ui_node_t *self, painter_device_t display) {
    fw_args_t *args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_FIRMWARE_ID_REDRAW_INTERVAL))) {
        return;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return;
    }

    const u128 id = get_build_id();

    // TODO: better handling of this
    // id is too long, copy only first few bytes
    string_t str = str_new(30);
    str_append(&str, "0x");
    for (size_t i = 0; i < 3; ++i) {
        str_printf(&str, "%x", id.bytes[i]);
    }
    str_append(&str, "...");

    const uint16_t width = qp_textwidth(font, str.ptr);
    if (width == 0 || width > self->size.x) {
        goto exit;
    }

    qp_drawtext(display, self->start.x, self->start.y, font, str.ptr);

exit:
    qp_close_font(font);
}

bool fw_sync_init(ui_node_t *self) {
    fw_args_t *args = self->args;

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return false;
    }

    const uint8_t line_height = font->line_height;
    qp_close_font(font);

    return line_height <= self->size.y;
}

void fw_sync_render(const ui_node_t *self, painter_device_t display) {
    fw_args_t *args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_FIRMWARE_SYNC_REDRAW_INTERVAL))) {
        return;
    }

    const u128 id = get_build_id();

    u128 slave_id;
    if (!get_slave_build_id(&slave_id)) {
        return;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return;
    }

    const bool        mismatch = memcmp(&id, &slave_id, sizeof(u128)) != 0;
    const char *const str      = (mismatch) ? "fw mismatch" : "fw matches";

    const uint16_t width = qp_textwidth(font, str);
    if (width == 0 || width > self->size.x) {
        goto exit;
    }

    const hsv_t fg = (mismatch) ? (hsv_t){HSV_RED} : (hsv_t){HSV_GREEN};
    qp_drawtext_recolor(display, self->start.x, self->start.y, font, str, fg.h, fg.s, fg.v, HSV_BLACK);

exit:
    qp_close_font(font);
}
