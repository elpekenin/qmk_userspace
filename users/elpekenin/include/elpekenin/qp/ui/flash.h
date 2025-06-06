// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/ui/common.h"

typedef struct {
    const uint8_t *font;
    uint32_t       timer;
    size_t         last;
} flash_args_t;
STATIC_ASSERT(offsetof(flash_args_t, font) == 0, "UI will crash :)");

bool flash_init(ui_node_t *self);
void flash_render(const ui_node_t *self, painter_device_t display);
