// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/compiler_support.h>

#include "elpekenin/qp/assets.h"
#include "elpekenin/time.h"

STATIC_ASSERT(CM_ENABLED(UI), "Must enable 'elpekenin/ui'");
#include "elpekenin/ui.h"

bool task_should_draw(uint32_t *last, uint32_t ms);

// TODO: remove  everything below

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
