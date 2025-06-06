// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include "elpekenin/qp/ui/common.h" // qp_callback_args_t

//
// sendchar
//
void   sendchar_qp_init(void);
int8_t sendchar_qp(uint8_t chr);

//
// ui rendering
//
typedef struct {
    const uint8_t *font;
    uint32_t       timer;
    uint32_t       last;
} qp_logging_args_t;
STATIC_ASSERT(offsetof(qp_logging_args_t, font) == 0, "UI will crash :)");

bool qp_logging_init(ui_node_t *self);
void qp_logging_render(const ui_node_t *self, painter_device_t display);

//
// utilities
//
void qp_log_clear(void);
void qp_logging_backend_render(qp_callback_args_t *args);
