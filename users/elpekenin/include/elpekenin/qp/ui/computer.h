// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/ui.h"

typedef struct {
    ui_time_t last_draw;
    bool      clear;
} computer_args_t;

bool      computer_init(ui_node_t *self);
ui_time_t computer_render(const ui_node_t *self, painter_device_t display);

void push_computer(uint8_t cpu, uint8_t ram);
