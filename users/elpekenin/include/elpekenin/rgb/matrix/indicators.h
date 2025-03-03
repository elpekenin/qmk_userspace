// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/color.h>

#include "elpekenin/utils/compiler.h"

// *** Types ***

// arguments passed from rgb_matrix_indicators to check function
typedef struct PACKED {
    uint8_t  led_index;
    uint8_t  layer;
    uint8_t  mods;
    uint16_t keycode;
} indicator_fn_args_t;

// flags
enum _indicator_flags_t {
    _LAYER,
    _KEYCODE,
    _MODS,       // mod(s) active, **not** exact match
    _KC_GT_THAN, // keycode greater than (eg for non-transparent keys, or custom keycodes)
    _LAST,
};
typedef uint8_t indicator_flags_t;
_Static_assert((indicator_flags_t)(1 << _LAST) != 0, "Check out the type for `indicators_flag_t`");

#define FLAG(flag) ((indicator_flags_t)(1 << flag))
#define LAYER FLAG(_LAYER)
#define KEYCODE FLAG(_KEYCODE)
#define MODS FLAG(_MODS)
#define KC_GT_THAN FLAG(_KC_GT_THAN)

// indicator specification: checks to perform (and their values) and color to draw
typedef struct PACKED {
    rgb_t               color;
    indicator_flags_t   flags;
    indicator_fn_args_t conditions;
} indicator_t;

// *** Macros ***

#define KEYCODE_INDICATOR(_keycode, rgb)  \
    (indicator_t) {                       \
        .color = {rgb}, .flags = KEYCODE, \
        .conditions = {                   \
            .keycode = _keycode,          \
        },                                \
    }

#define LAYER_INDICATOR(_layer, rgb)    \
    (indicator_t) {                     \
        .color = {rgb}, .flags = LAYER, \
        .conditions = {                 \
            .layer = _layer,            \
        },                              \
    }

#define KEYCODE_IN_LAYER_INDICATOR(_keycode, _layer, rgb) \
    (indicator_t) {                                       \
        .color = {rgb}, .flags = KEYCODE | LAYER,         \
        .conditions = {                                   \
            .keycode = _keycode,                          \
            .layer   = _layer,                            \
        },                                                \
    }

#define NO_TRANS_KEYCODE_IN_LAYER_INDICATOR(_keycode, _layer, rgb) \
    (indicator_t) {                                                \
        .color = {rgb}, .flags = LAYER | KC_GT_THAN,               \
        .conditions = {                                            \
            .keycode = _keycode,                                   \
            .layer   = _layer,                                     \
        },                                                         \
    }

#define KEYCODE_WITH_MOD_INDICATOR(_keycode, mod_mask, rgb) \
    (indicator_t) {                                         \
        .color = {rgb}, .flags = KEYCODE | MODS,            \
        .conditions = {                                     \
            .keycode = _keycode,                            \
            .mods    = mod_mask,                            \
        },                                                  \
    }

#define CUSTOM_KEYCODE_IN_LAYER_INDICATOR(_layer, rgb) \
    (indicator_t) {                                    \
        .color = {rgb}, .flags = LAYER | KC_GT_THAN,   \
        .conditions = {                                \
            .keycode = QK_USER,                        \
            .layer   = _layer,                         \
        },                                             \
    }

// *** Colors ***

#define MAX_WHITE                                                                                   \
    (rgb_t) {                                                                                       \
        RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS \
    }

// *** API ***

bool draw_indicators(uint8_t led_min, uint8_t led_max);
