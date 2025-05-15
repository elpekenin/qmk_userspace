// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/painter/qp.h>
#include <quantum/util.h>

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

// show build info
void draw_commit(painter_device_t device);
void draw_features(painter_device_t device); // generated/features_draw

// configure tasks
qp_callback_args_t *get_logging_args(void);
qp_callback_args_t *get_uptime_args(void);
qp_callback_args_t *get_layer_args(void);
qp_callback_args_t *get_heap_stats_args(void);
qp_callback_args_t *get_keylog_args(void);
qp_callback_args_t *get_computer_stats_args(void);

void push_computer_stats(uint8_t cpu, uint8_t ram);
