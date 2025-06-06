// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/ui/common.h"

typedef struct {
    const uint8_t *logo;
    uint32_t       timer;
    size_t         last;
    bool           clear;
} github_notifications_args_t;
STATIC_ASSERT(offsetof(github_notifications_args_t, logo) == 0, "UI will crash :)");

bool github_notifications_init(ui_node_t *self);
void github_notifications_render(const ui_node_t *self, painter_device_t display);

void set_github_notifications_count(uint8_t count);
