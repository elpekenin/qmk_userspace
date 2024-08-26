// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/action_layer.h>
#include <quantum/led.h>

#include "elpekenin/utils/compiler.h"

WEAK void housekeeping_task_keymap(void);
WEAK void keyboard_pre_init_keymap(void);
WEAK void keyboard_post_init_keymap(void);
WEAK layer_state_t layer_state_set_keymap(layer_state_t state);
WEAK bool led_update_keymap(led_t led_state);
NON_NULL(2) READ_ONLY(2) WEAK bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
WEAK bool shutdown_keymap(bool jump_to_bootloader);
WEAK void suspend_power_down_keymap(void);
WEAK void suspend_wakeup_init_keymap(void);
