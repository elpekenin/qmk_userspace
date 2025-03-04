// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/action_layer.h>
#include <quantum/action_util.h>
#include <quantum/keymap_common.h>
#include <platforms/progmem.h>
#include "default_keyboard.h" // for LAYOUT

#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/ledmap.h"
#include "elpekenin/rgb/matrix/indicators.h"

// *** Definitions ***

static indicator_t indicators[] = {
    LAYER_INDICATOR(_RST, RGB_OFF),

    // QMK keycodes
    KEYCODE_IN_LAYER_INDICATOR(QK_BOOT, _RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(QK_RBT, _RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(EE_CLR, _RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(DB_TOGG, _RST, RGB_RED),
    // KEYCODE_IN_LAYER_INDICATOR(AC_DICT, _RST, RGB_RED),

    // custom keycodes
    CUSTOM_KEYCODE_IN_LAYER_INDICATOR(_RST, RGB_BLUE),
};

static void handle_indicator(const indicator_t *indicator, const indicator_fn_args_t *args) {
    if (indicator->flags & LAYER && indicator->conditions.layer != args->layer) {
        return;
    }

    if (indicator->flags & KEYCODE && indicator->conditions.keycode != args->keycode) {
        return;
    }

    if (indicator->flags & MODS && !(indicator->conditions.mods & args->mods)) {
        return;
    }

    if (indicator->flags & KC_GT_THAN && indicator->conditions.keycode >= args->keycode) {
        return;
    }

    rgb_matrix_set_color(args->led_index, indicator->color.r, indicator->color.g, indicator->color.b);
}

// *** Callback ***

bool draw_indicators(uint8_t led_min, uint8_t led_max) {
    uint8_t mods  = get_mods();
    uint8_t layer = get_highest_layer(layer_state);

    indicator_fn_args_t args = {
        .mods  = mods,
        .layer = layer,
    };

    // iterate all keys
    for (int8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (int8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t index = g_led_config.matrix_co[row][col];

            // early exit if out of range
            if (index < led_min || index >= led_max) {
                continue;
            }

            args.led_index = index;
            args.keycode   = keymap_key_to_keycode(layer, (keypos_t){col, row});

            // iterate all indicators
            for (int8_t i = 0; i < ARRAY_SIZE(indicators); ++i) {
                const indicator_t *indicator = &indicators[i];
                // conditionally draws
                handle_indicator(indicator, &args);
            }
        }
    }

    return false;
}
