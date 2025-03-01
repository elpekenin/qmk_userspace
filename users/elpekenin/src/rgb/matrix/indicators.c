// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/action_layer.h>
#include <quantum/action_util.h>
#include <quantum/keymap_common.h>
#include <platforms/progmem.h>
#include "default_keyboard.h" // for LAYOUT

#include "elpekenin/errno.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
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

// NOTES:
//   - Assumes (for now?) that all LEDs are mapped to a key (no underglow or w/e)
//   - Available colors defined on the `.h` file
//   - TRNS on layer 0 => nothing drawn (respects animation)
//   - Undefined layers => same as above
static const uint8_t PROGMEM ledmap[][MATRIX_ROWS][MATRIX_COLS] = {
    // [_QWERTY] = LAYOUT(
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,    BLACK,         WHITE,    RED,  TRNS, RED,  RED
    // ),
    // [_FN1] = LAYOUT(
    //     TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
    //     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,    CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,
    //     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    //     ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,    ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,
    //     WHITE,WHITE,BLACK,TRNS,    BLACK,         BLACK,   RED,  TRNS, WHITE,WHITE
    // )
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

// *** Ledmap ***

#define LEDMAP_LAYERS (sizeof(ledmap) / (MATRIX_ROWS * MATRIX_COLS))

CONST static inline bool is_special_color(uint8_t hue) {
    return hue >= _MARKER_;
};

NON_NULL(4) WRITE_ONLY(4) static inline int get_ledmap_color(uint8_t layer, uint8_t row, uint8_t col, rgb_t *rgb) {
    if (layer >= LEDMAP_LAYERS) {
        return -EINVAL;
    }

    uint8_t hue = pgm_read_byte(&(ledmap[layer][row][col]));
    uint8_t sat = rgb_matrix_get_sat();
    uint8_t val = rgb_matrix_get_val();
    hsv_t   hsv = {hue, sat, val};

    // not "regular" colors (hue), but ones with special handling
    if (is_special_color(hue)) {
        switch (hue) {
            case TRNS:
                if (layer == 0) {
                    return -EINVAL;
                }

                // look up further down (only on active layers)
                for (int8_t __layer = layer - 1; __layer > 0; --__layer) {
                    if (layer_state & (1 << __layer)) {
                        return get_ledmap_color(layer - 1, row, col, rgb);
                    }
                }

                return -ENOTFOUND;

            case WHITE:
                hsv = (hsv_t){0, 0, val};
                break;

            case BLACK:
                hsv = (hsv_t){0, 0, 0};
                break;
        }
    }

    *rgb = hsv_to_rgb(hsv);
    return 0;
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

            // draw ledmap color
            rgb_t rgb;
            if (get_ledmap_color(layer, row, col, &rgb) == 0) {
                rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
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
