// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/ui.h"

typedef struct {
    const uint8_t *logo;
    uint32_t       timer;
    uint32_t       last;
    bool           clear;
} github_args_t;
STATIC_ASSERT(offsetof(github_args_t, logo) == 0, "UI will crash :)");

bool     github_init(ui_node_t *self);
uint32_t github_render(const ui_node_t *self, painter_device_t display);

void set_github_count(uint8_t count);
