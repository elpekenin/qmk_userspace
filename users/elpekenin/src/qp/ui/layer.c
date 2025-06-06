// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO: glitch refactor

#include "elpekenin/qp/ui/layer.h"

#include <quantum/compiler_support.h>

#include "elpekenin/layers.h"

bool layer_init(ui_node_t *self) {
    layer_args_t *const args = self->args;

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return false;
    }

    const uint8_t line_height = font->line_height;
    qp_close_font(font);

    if (line_height > self->size.y) {
        return false;
    }

    args->last.layer = ~0;
    return true;
}

void layer_render(const ui_node_t *self, painter_device_t display) {
    layer_args_t *const args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_LAYER_REDRAW_INTERVAL))) {
        return;
    }

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    if (args->last.layer == layer) {
        return;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        return;
    }

    const char *const str = get_layer_name(layer);

    const uint16_t width = qp_textwidth(font, str);
    if (width == 0 || width > self->size.x) {
        goto exit;
    }

    if (args->last.width > width) {
        bool ret = qp_rect(display, self->start.x + width, self->start.y, self->start.x + args->last.width, self->start.y + font->line_height, HSV_BLACK, true);
        if (!ret) {
            goto exit;
        }
    }

    qp_drawtext(display, self->start.x, self->start.y, font, str);

    args->last = (last_layer_t){
        .layer = layer,
        .width = width,
    };

exit:
    qp_close_font(font);
}
