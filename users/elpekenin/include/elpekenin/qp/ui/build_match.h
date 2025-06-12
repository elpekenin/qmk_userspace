// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/ui.h"

typedef struct {
    const uint8_t *font;
} build_match_args_t;
STATIC_ASSERT(offsetof(build_match_args_t, font) == 0, "UI will crash :)");

bool     build_match_init(ui_node_t *self);
uint32_t build_match_render(const ui_node_t *self, painter_device_t display);
