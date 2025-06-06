// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/ui/common.h"

typedef struct {
    const uint8_t *font;
    uint32_t       timer;
} uptime_args_t;
STATIC_ASSERT(offsetof(uptime_args_t, font) == 0, "UI will crash :)");

bool uptime_init(ui_node_t *self);
void uptime_render(const ui_node_t *self, painter_device_t display);
