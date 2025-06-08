// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/build_match.h"

#include "elpekenin/split/transactions.h"
#include "elpekenin/ui/utils.h"

bool build_match_init(ui_node_t *self) {
    return ui_font_fits(self);
}

uint32_t build_match_render(const ui_node_t *self, painter_device_t display) {
    build_match_args_t *args = self->args;

    const u128 id = get_build_id();

    u128 slave_id;
    if (!get_slave_build_id(&slave_id)) {
        goto exit;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        goto exit;
    }

    const bool        mismatch = memcmp(&id, &slave_id, sizeof(u128)) != 0;
    const char *const str      = (mismatch) ? "fw mismatch" : "fw matches";

    const uint16_t width = qp_textwidth(font, str);
    if (width == 0 || width > self->size.x) {
        goto err;
    }

    const hsv_t fg = (mismatch) ? (hsv_t){HSV_RED} : (hsv_t){HSV_GREEN};
    qp_drawtext_recolor(display, self->start.x, self->start.y, font, str, fg.h, fg.s, fg.v, HSV_BLACK);

err:
    qp_close_font(font);

exit:
    return BUILD_MATCH_UI_REDRAW_INTERVAL;
}
