// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <sys/cdefs.h>

#include "elpekenin/signatures.h"

__weak_symbol void housekeeping_task_keymap(void) {}
__weak_symbol void keyboard_pre_init_keymap(void) {}
__weak_symbol void keyboard_post_init_keymap(void) {}

__weak_symbol layer_state_t layer_state_set_keymap(layer_state_t state) {
    return state;
}

__weak_symbol bool led_update_keymap(__unused led_t led_state) {
    return true;
}

__weak_symbol bool process_record_keymap(__unused uint16_t keycode, __unused keyrecord_t *record) {
    return true;
}
__weak_symbol bool shutdown_keymap(__unused bool jump_to_bootloader) {
    return true;
}
__weak_symbol void suspend_power_down_keymap(void) {}
__weak_symbol void suspend_wakeup_init_keymap(void) {}

__weak_symbol bool rgb_matrix_indicators_advanced_keymap(__unused uint8_t led_min, __unused uint8_t led_max) {
    return true;
}
