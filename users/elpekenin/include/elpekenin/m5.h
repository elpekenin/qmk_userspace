// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Add IoT capabilities to our keyboard over UART :)
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>

#include "elpekenin/touch.h"

void m5_init(void);

void m5_mic_start(void);
void m5_mic_stop(void);

void m5_mqtt_screen_pressed(uint8_t screen_id, touch_report_t report);
void m5_mqtt_screen_released(uint8_t screen_id);
void m5_mqtt_layer(layer_state_t layer_state);
void m5_mqtt_keyevent(uint16_t keycode, keyrecord_t *record);
