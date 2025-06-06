// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/github_notifications.h"

#include "elpekenin/xap.h"

static struct {
    size_t   count;
    uint32_t last;
} state = {0};

bool github_notifications_init(ui_node_t *self) {
    github_notifications_args_t *const args = self->args;

    const painter_image_handle_t image = qp_load_image_mem(args->logo);
    if (image == NULL) {
        return false;
    }

    const uint16_t image_width  = image->width;
    const uint16_t image_height = image->height;
    qp_close_image(image);

    if (image_width > self->size.x || image_height > self->size.y) {
        return false;
    }

    return true;
}

void github_notifications_render(const ui_node_t *self, painter_device_t display) {
    github_notifications_args_t *const args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_GITHUB_NOTIFICATIONS_REDRAW_INTERVAL))) {
        return;
    }

    const painter_image_handle_t image = qp_load_image_mem(args->logo);
    if (image == NULL) {
        return;
    }

    // clear after inactivity
    if (xap_last_activity_elapsed() > MILLISECONDS(QP_TASK_GITHUB_NOTIFICATIONS_TIMEOUT)) {
        if (args->clear != true) {
            args->clear = false;
            qp_rect(display, self->start.x, self->start.y, self->start.x + image->width, self->start.y + image->height, HSV_BLACK, true);
        }

        goto exit;
    }

    if (args->last <= state.last) {
        goto exit;
    }

    hsv_t fg;
    switch (state.count) {
        case 0:
            fg = (hsv_t){HSV_WHITE};
            break;

        case 1 ... 3:
            fg = (hsv_t){HSV_GREEN};
            break;

        case 4 ... 7:
            fg = (hsv_t){HSV_ORANGE};
            break;

        default:
            fg = (hsv_t){HSV_RED};
            break;
    }

    qp_drawimage_recolor(display, self->start.x, self->start.y, image, fg.h, fg.s, fg.v, HSV_BLACK);

    args->last  = timer_read32();
    args->clear = true;

exit:
    qp_close_image(image);
}

void set_github_notifications_count(uint8_t count) {
    state.count = count;
    state.last  = timer_read32();
}
