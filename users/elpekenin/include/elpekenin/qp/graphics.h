// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

// *** Show build info ***

void draw_commit(painter_device_t device);
void draw_features(painter_device_t device);

// *** Scrolling text ***

typedef struct PACKED {
    painter_device_t      device;
    uint16_t              x;
    uint16_t              y;
    painter_font_handle_t font;
    char                 *str;
    uint8_t               n_chars; // Chars being drawn each time
    uint32_t              delay;
    uint8_t               char_number; // Current position
    uint8_t               spaces;      // Whitespaces between string repetition
    int16_t               width;       // of the currently-shown string, to remove before rendering
    qp_pixel_t            fg;
    qp_pixel_t            bg;
    deferred_token        defer_token;
} scrolling_text_state_t;

deferred_token draw_scrolling_text(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay);

deferred_token draw_scrolling_text_recolor(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg);

void stop_scrolling_text(deferred_token scrolling_token);

void extend_scrolling_text(deferred_token scrolling_token, const char *str);

// *** Callbacks ***

void set_logging_device(painter_device_t device);
void set_uptime_device(painter_device_t device);
void set_heap_stats_device(painter_device_t device);
void set_layer_device(painter_device_t device);

#if defined(KEYLOG_ENABLE)
void set_keylog_device(painter_device_t device);
#endif

// exposed for logging/backend/qp.c
typedef struct PACKED {
    uint8_t  n_chars;
    uint32_t delay;
} scrolling_args_t;

typedef struct PACKED {
    painter_device_t      device;
    painter_font_handle_t font;
    scrolling_args_t      scrolling_args;
    uint16_t              x;
    uint16_t              y;
    void                 *extra;
} qp_callback_args_t;
