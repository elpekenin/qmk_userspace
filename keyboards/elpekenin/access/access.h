// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/painter/qp.h>

#include "elpekenin/touch.h"

extern painter_device_t il91874;
extern painter_device_t ili9163;
extern painter_device_t ili9341;

extern touch_device_t ili9341_touch;

bool is_ili9341_pressed(void);
