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

    u128 id;
    if (get_build_id(&id) < 0) {
        return UI_STOP;
    }

    const painter_font_handle_t font = qp_load_font_mem(args->font);
    if (font == NULL) {
        goto exit;
    }

    if (!is_keyboard_master()) {
        const char *const str = "unknown";

        if (ui_text_fits(self, font, str)) {
            qp_drawtext_recolor(display, self->start.x, self->start.y, font, str, HSV_YELLOW, HSV_BLACK);
        }

        goto err;
    }

    u128 slave_id;
    if (!get_slave_build_id(&slave_id)) {
        const char *const str = "comms fail";

        if (ui_text_fits(self, font, str)) {
            qp_drawtext_recolor(display, self->start.x, self->start.y, font, str, HSV_ORANGE, HSV_BLACK);
        }

        goto err;
    }

    const bool        mismatch = memcmp(&id, &slave_id, sizeof(u128)) != 0;
    const char *const str      = (mismatch) ? "fw mismatch" : "fw matches";

    if (ui_text_fits(self, font, str)) {
        const hsv_t fg = (mismatch) ? (hsv_t){HSV_RED} : (hsv_t){HSV_GREEN};

        qp_drawtext_recolor(display, self->start.x, self->start.y, font, str, fg.h, fg.s, fg.v, HSV_BLACK);
    }

err:
    qp_close_font(font);

exit:
    return BUILD_MATCH_UI_REDRAW_INTERVAL;
}
