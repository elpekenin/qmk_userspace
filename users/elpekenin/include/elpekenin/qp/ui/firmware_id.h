// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/ui/common.h"

typedef struct {
    const uint8_t *font;
    uint32_t       timer;
} fw_args_t;
STATIC_ASSERT(offsetof(fw_args_t, font) == 0, "UI will crash :)");

bool fw_id_init(ui_node_t *self);
void fw_id_render(const ui_node_t *self, painter_device_t display);

bool fw_sync_init(ui_node_t *self);
void fw_sync_render(const ui_node_t *self, painter_device_t display);
