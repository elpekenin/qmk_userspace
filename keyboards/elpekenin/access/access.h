// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp.h"

#    if IS_LEFT_HAND
extern painter_device_t il91874;
#    endif

#    if IS_RIGHT_HAND
extern painter_device_t ili9163;
extern painter_device_t ili9341;
#    endif

#endif

#if defined(QUANTUM_PAINTER_ENABLE) && defined(TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
#    include "elpekenin/touch.h"
extern touch_device_t ili9341_touch;

bool is_ili9341_pressed(void);
#endif
