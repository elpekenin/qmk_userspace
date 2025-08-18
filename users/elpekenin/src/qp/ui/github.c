// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/github.h"

#include "elpekenin/ui/utils.h"
#include "elpekenin/xap.h"

static struct {
    size_t    count;
    ui_time_t last_update;
} state = {0};

bool github_init(ui_node_t *self) {
    return ui_image_fits(self);
}

ui_time_t github_render(const ui_node_t *self, painter_device_t display) {
    github_args_t *const args = self->args;

    const painter_image_handle_t image = qp_load_image_mem(args->logo);
    if (image == NULL) {
        goto exit;
    }

    // clear after inactivity
    if (xap_last_activity_elapsed() > GITHUB_NOTIFICATIONS_UI_TIMEOUT) {
        if (args->clear) {
            args->clear = false;
            qp_rect(display, self->start.x, self->start.y, self->start.x + image->width, self->start.y + image->height, HSV_BLACK, true);
        }

        goto err;
    }

    if (ui_time_lte(state.last_update, args->last_draw)) {
        goto err;
    }

    hsv_t fg = {HSV_BLACK};
    switch (state.count) {
        case 0:
            fg = (hsv_t){HSV_WHITE};
            break;

        case 1:
            fg = (hsv_t){HSV_GREEN};
            break;

        case 2:
            fg = (hsv_t){HSV_ORANGE};
            break;

        default:
            fg = (hsv_t){HSV_RED};
            break;
    }

    qp_drawimage_recolor(display, self->start.x, self->start.y, image, fg.h, fg.s, fg.v, HSV_BLACK);

    args->last_draw = ui_time_now();
    args->clear     = true;

err:
    qp_close_image(image);

exit:
    return UI_MILLISECONDS(GITHUB_NOTIFICATIONS_UI_REDRAW_INTERVAL);
}

void set_github_count(uint8_t count) {
    state.count       = count;
    state.last_update = ui_time_now();
}
