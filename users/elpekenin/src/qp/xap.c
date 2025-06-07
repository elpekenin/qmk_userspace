// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO
// - Maybe the XAP answer should be success/failure based on the output from QP functions, returning success for now as in "your request has been handled"

#include "elpekenin/xap.h"

#include <quantum/quantum.h>
#include <quantum/xap/xap.h>

#include "elpekenin/qp/assets.h"
#include "elpekenin/qp/ui/computer.h"
#include "elpekenin/qp/ui/github.h"
#include "elpekenin/scrolling_text.h"
#include "elpekenin/split/transactions.h"

static inline uint8_t lsb(uint16_t val) {
    return val & 0xFF;
}

static inline uint8_t msb(uint16_t val) {
    return (val >> 8) & 0xFF;
}

static inline void xap_slave(const void *data) {
    // offset backwards into start of the XAP message
    // { user, qp, operation } = 3 bytes to offset
    const void *start = data - sizeof(xap_request_header_t) - 3;
    xap_execute_slave(start);
}

static inline void xap_preprocess(xap_token_t token) {
    xap_respond_success(token);
    xap_last_activity_update();
}

//
// QP control
//

bool xap_execute_qp_clear(xap_token_t token, xap_route_user_quantum_painter_clear_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_clear(device);
    }

    return true;
}

bool xap_execute_qp_setpixel(xap_token_t token, xap_route_user_quantum_painter_setpixel_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_setpixel(device, arg->x, arg->y, arg->hue, arg->sat, arg->val);
    }

    return true;
}

bool xap_execute_qp_line(xap_token_t token, xap_route_user_quantum_painter_draw_line_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_line(device, arg->x0, arg->y0, arg->x1, arg->y1, arg->hue, arg->sat, arg->val);
    }

    return true;
}

bool xap_execute_qp_rect(xap_token_t token, xap_route_user_quantum_painter_draw_rect_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_rect(device, arg->left, arg->top, arg->right, arg->bottom, arg->hue, arg->sat, arg->val, arg->filled);
    }

    return true;
}

bool xap_execute_qp_circle(xap_token_t token, xap_route_user_quantum_painter_draw_circle_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_circle(device, arg->x, arg->y, arg->radius, arg->hue, arg->sat, arg->val, arg->filled);
    }

    return true;
}

bool xap_execute_qp_ellipse(xap_token_t token, xap_route_user_quantum_painter_draw_ellipse_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_ellipse(device, arg->x, arg->y, arg->sizex, arg->sizey, arg->hue, arg->sat, arg->val, arg->filled);
    }

    return true;
}

bool xap_execute_qp_drawimage(xap_token_t token, xap_route_user_quantum_painter_drawimage_arg_t *arg) {
    xap_preprocess(token);

    const painter_image_handle_t image = get_image_by_name((const char *)arg->image_name);
    if (image == NULL) {
        printf("Unknown image: %s\n", arg->image_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_drawimage(device, arg->x, arg->y, image);
    }

    return true;
}

bool xap_execute_qp_drawimage_recolor(xap_token_t token, xap_route_user_quantum_painter_drawimage_recolor_arg_t *arg) {
    xap_preprocess(token);

    const painter_image_handle_t image = get_image_by_name((const char *)arg->image_name);
    if (image == NULL) {
        printf("Unknown image: %s\n", arg->image_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_drawimage_recolor(device, arg->x, arg->y, image, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);
    }

    return true;
}

bool xap_execute_qp_animate(xap_token_t token, xap_route_user_quantum_painter_animate_arg_t *arg) {
    xap_preprocess(token);

    const painter_image_handle_t image = get_image_by_name((const char *)arg->image_name);
    if (image == NULL) {
        printf("Unknown image: %s\n", arg->image_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_animate(device, arg->x, arg->y, image);
    }

    return true;
}

bool xap_execute_qp_animate_recolor(xap_token_t token, xap_route_user_quantum_painter_animate_recolor_arg_t *arg) {
    xap_preprocess(token);

    const painter_image_handle_t image = get_image_by_name((const char *)arg->image_name);
    if (image == NULL) {
        printf("Unknown image: %s\n", arg->image_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_animate_recolor(device, arg->x, arg->y, image, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);
    }

    return true;
}

bool xap_execute_qp_drawtext(xap_token_t token, xap_route_user_quantum_painter_drawtext_arg_t *arg) {
    xap_preprocess(token);

    const painter_font_handle_t font = get_font_by_name((const char *)arg->font_name);
    if (font == NULL) {
        printf("Unknown font: %s\n", arg->font_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_drawtext(device, arg->x, arg->y, font, (const char *)arg->text);
    }

    return true;
}

bool xap_execute_qp_drawtext_recolor(xap_token_t token, xap_route_user_quantum_painter_drawtext_recolor_arg_t *arg) {
    xap_preprocess(token);

    const painter_font_handle_t font = get_font_by_name((const char *)arg->font_name);
    if (font == NULL) {
        printf("Unknown font: %s\n", arg->font_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_drawtext_recolor(device, arg->x, arg->y, font, (const char *)arg->text, arg->hue_fg, arg->sat_fg, arg->val_fg, arg->hue_bg, arg->sat_bg, arg->val_bg);
    }

    return true;
}

bool xap_execute_qp_get_geometry(xap_token_t token, xap_route_user_quantum_painter_get_geometry_arg_t *arg) {
    xap_preprocess(token);

    painter_rotation_t rotation;
    uint16_t           width;
    uint16_t           height;
    uint16_t           offset_x;
    uint16_t           offset_y;

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_get_geometry(device, &width, &height, &rotation, &offset_x, &offset_y);

        uint8_t ret[9] = {lsb(width), msb(width), lsb(height), msb(height), rotation, lsb(offset_x), msb(offset_x), lsb(offset_y), msb(offset_y)};
        xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)ret, sizeof(ret));
    }

    return true;
}

bool xap_execute_qp_flush(xap_token_t token, xap_route_user_quantum_painter_flush_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_flush(device);
    }

    return true;
}

bool xap_execute_qp_viewport(xap_token_t token, xap_route_user_quantum_painter_viewport_arg_t *arg) {
    xap_preprocess(token);

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_viewport(device, arg->left, arg->top, arg->right, arg->bottom);
    }

    return true;
}

bool xap_respond_qp_pixdata(xap_token_t token, const uint8_t *data, size_t data_len) {
    xap_preprocess(token);

    const xap_route_user_quantum_painter_pixdata_arg_t *arg = (void *)data;

    const uint8_t device_name_len = 10;
    const uint8_t n_pixels        = (data_len - device_name_len) / 2;

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        qp_pixdata(device, (const void *)arg->pixels, n_pixels);
    }

    return true;
}

bool xap_execute_qp_textwidth(xap_token_t token, xap_route_user_quantum_painter_textwidth_arg_t *arg) {
    xap_preprocess(token);

    const painter_font_handle_t font = get_font_by_name((const char *)arg->font_name);
    if (font == NULL) {
        printf("Unknown font: %s\n", arg->font_name);
        return true;
    }

    int16_t width = qp_textwidth(font, (const char *)arg->text);

    uint8_t ret[2] = {lsb(width), msb(width)};
    xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)ret, sizeof(ret));

    return true;
}

bool xap_execute_draw_scrolling_text(xap_token_t token, xap_route_user_quantum_painter_scrolling_text_arg_t *arg) {
    xap_preprocess(token);

    const painter_font_handle_t font = get_font_by_name((const char *)arg->font_name);
    if (font == NULL) {
        printf("Unknown font: %s\n", arg->font_name);
        return true;
    }

    const painter_device_t device = get_device_by_name((const char *)arg->device_name);
    if (device == NULL) {
        xap_slave(arg);
    } else {
        const scrolling_text_config_t config = {
            .device  = device,
            .x       = arg->x,
            .y       = arg->y,
            .font    = font,
            .n_chars = arg->n_chars,
            .delay   = arg->delay,
            .spaces  = 5,
            .bg      = {HSV_BLACK},
            .fg      = {HSV_WHITE},
#if SCROLLING_TEXT_USE_ALLOCATOR
            .allocator = c_runtime_allocator,
#endif
        };

        const deferred_token def_token = scrolling_text_start(&config, (char *)arg->text);

        xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, &def_token, sizeof(deferred_token));
    }

    return true;
}

bool xap_execute_stop_scrolling_text(xap_token_t token, xap_route_user_quantum_painter_stop_scrolling_text_arg_t *arg) {
    xap_preprocess(token);
    scrolling_text_stop(arg->token);
    return true;
}

bool xap_execute_extend_scrolling_text(xap_token_t token, xap_route_user_quantum_painter_extend_scrolling_text_arg_t *arg) {
    xap_preprocess(token);
    scrolling_text_extend(arg->token, (const char *)arg->text);
    return true;
}

//
// tasks
//

bool xap_execute_push_computer(xap_token_t token, xap_route_user_tasks_push_computer_arg_t *arg) {
    xap_preprocess(token);
    push_computer(arg->cpu, arg->ram);
    return true;
}

bool xap_execute_set_github_count(xap_token_t token, xap_route_user_tasks_set_github_count_arg_t *arg) {
    xap_preprocess(token);
    set_github_count(arg->count);
    return true;
}
