// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

// *** Show build info ***

void draw_commit(painter_device_t device);
void draw_features(painter_device_t device);

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
