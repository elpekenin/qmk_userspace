// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/color.h>

#include "elpekenin/utils/compiler.h"

// *** Types ***

// arguments passed from rgb_matrix_indicators to check function
typedef struct PACKED {
    uint8_t led_index;
    uint8_t layer;
    uint8_t mods;
    uint16_t keycode;
} indicator_fn_args_t;

// flags
#define LAYER      (1 << 0)
#define KEYCODE    (1 << 1)
#define MODS       (1 << 2) // mod(s) active, **not** exact match
#define KC_GT_THAN (1 << 3) // keycode greater than (eg for non-transparent keys, or custom keycodes)
typedef uint8_t indicator_flags_t;

// indicator specification: checks to perform (and their values) and color to draw
typedef struct PACKED  {
    rgb_led_t color;
    indicator_flags_t flags;
    indicator_fn_args_t conditions;
} indicator_t;

// *** Macros ***

// this crap is needed for RGB color macros to work nicely...
// relying on position aka: (rgb_led_t){_col} doesnt work
static inline const rgb_led_t __rgb(uint8_t redish, uint8_t greenish, uint8_t blueish) {
    return (rgb_led_t) {.r = redish, .g = greenish, .b = blueish};
}

static inline const indicator_t keycode_indicator(uint16_t keycode, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = KEYCODE,
        .conditions = {
            .keycode = keycode,
        },
    };
}

static inline const indicator_t layer_indicator(uint8_t layer, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = LAYER,
        .conditions = {
            .layer = layer,
        },
    };
}

static inline const indicator_t keycode_in_layer_indicator(uint16_t keycode, uint8_t layer, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = KEYCODE | LAYER,
        .conditions = {
            .keycode = keycode,
            .layer = layer,
        },
    };
}

static inline const indicator_t no_trans_keycode_in_layer_indicator(uint16_t keycode, uint8_t layer, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = LAYER | KC_GT_THAN,
        .conditions = {
            .keycode = keycode,
            .layer = layer,
        },
    };
}

static inline const indicator_t keycode_with_mod_indicator(uint16_t keycode, uint8_t mod_mask, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = KEYCODE | MODS,
        .conditions = {
            .keycode = keycode,
            .mods = mod_mask,
        },
    };
}

static inline const indicator_t custom_keycode_in_layer_indicator(uint8_t layer, uint8_t r, uint8_t g, uint8_t b) {
    return (indicator_t) {
        .color = __rgb(r, g, b),
        .flags = LAYER | KC_GT_THAN,
        .conditions = {
            .keycode = QK_USER,
            .layer = layer,
        },
    };
}

// *** Colors ***

#define MAX_WHITE RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS

enum colors {
    RED = 0,
    ORANGE = 21,
    YELLOW = 43,
    CHARTREUSE = 64,
    GREEN = 85,
    SPRING = 106,
    CYAN = 127,
    AZURE = 148,
    BLUE = 169,
    VIOLET = 180,
    MAGENTA = 201,
    ROSE = 222,

    // special values
    TRNS = 253,
    WHITE = 254,
    BLACK = 255,

    // has to be equal to the smallest special value
    // ... or rework the `is_special_color` function
    _MARKER_ = TRNS,
};


// *** API ***

bool draw_indicators(uint8_t led_min, uint8_t led_max);
