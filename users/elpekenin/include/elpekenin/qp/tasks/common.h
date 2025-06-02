// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/painter/qp.h>
#include <quantum/util.h>

#include "elpekenin/time.h"

typedef struct PACKED {
    size_t   n_chars;
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
