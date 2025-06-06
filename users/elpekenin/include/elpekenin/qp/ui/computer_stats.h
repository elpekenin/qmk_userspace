// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/ui/common.h"

typedef struct {
    uint32_t timer;
    size_t   last;
    bool     clear;
} computer_stats_args_t;

bool computer_stats_init(ui_node_t *self);
void computer_stats_render(const ui_node_t *self, painter_device_t display);

void push_computer_stats(uint8_t cpu, uint8_t ram);
