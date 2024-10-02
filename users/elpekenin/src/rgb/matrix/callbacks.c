// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/layers.h"
#include "elpekenin/placeholders.h"
#include "elpekenin/rgb/matrix/functions.h"
#include "elpekenin/rgb/matrix/indicators.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/shortcuts.h"

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_keymap(led_min, led_max)) {
        return false;
    }

    return draw_indicators(led_min, led_max);
}

bool led_update_user(led_t led_state) {
    if (!led_update_keymap(led_state)) {
        return false;
    }

    // i dont really want debug here:
    //    - rgb matrix mode [NOEEPROM]: x
    //    - rgb matrix set hsv [NOEEPROM]: x, y, z
    // clang-format off
    WITHOUT_DEBUG(
        if (led_state.caps_lock) {
            rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
            rgb_matrix_sethsv_noeeprom(165, 255, rgb_matrix_get_val());
        } else {
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_LEFT_RIGHT);
        }
    );
    // clang-format on

    return false;
}

WEAK bool rgb_matrix_indicators_advanced_keymap(uint8_t led_min, uint8_t led_max) {
    return true;
}

void rgb_shutdown(bool jump_to_bootloader) {
    if (jump_to_bootloader) {
        // off for bootloader
        rgb_matrix_set_color_all(RGB_OFF);
    } else {
        // red for reboot
        rgb_matrix_set_color_all(RGB_MATRIX_MAXIMUM_BRIGHTNESS, 0, 0);
    }

    // flush
    void rgb_matrix_update_pwm_buffers(void);
    rgb_matrix_update_pwm_buffers();
}
PEKE_DEINIT(rgb_shutdown, DEINIT_RGB);
