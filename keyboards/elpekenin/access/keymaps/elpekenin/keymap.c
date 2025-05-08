// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdlib.h>

#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/qp/graphics.h"
#include "elpekenin/signatures.h"
#include "elpekenin/time.h"
#include "elpekenin/xap.h"

#if defined(COMMUNITY_MODULE_INDICATORS_ENABLE)
#    include "elpekenin/indicators.h"
#endif

#if defined(COMMUNITY_MODULE_LEDMAP_ENABLE)
#    include "elpekenin/ledmap.h"
#endif

#if defined(COMMUNITY_MODULE_MICROPYTHON_ENABLE)
#    include "port/micropython_embed.h"
#endif

#if defined(COMMUNITY_MODULE_RNG_ENABLE)
#    include <platforms/chibios/drivers/analog.h>

#    include "elpekenin/rng.h"
#endif

#if defined(COMMUNITY_MODULE_STRING_ENABLE)
#    include "elpekenin/string.h"
#else
#    error Must enable 'elpekenin/string'
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT(
        ESC,     N1,      N2,      N3,      N4,      N5,             N6,      N7,      N8,      N9,      N0,      BSPC,
        TAB,     Q,       W,       E,       R,       T,              Y,       U,       I,       O,       P,       PLUS,
        XXXXXXX, A,       S,       D,       F,       G,              H,       J,       K,       L,       TD_NTIL, XXXXXXX,
        SFT,     TD_Z,    X,       C,       V,       B,              N,       M,       COMM,    DOT,     MINS,    GRV,
        CTL,     GUI,     ALT,     TL_UPPR,   TD_SPC,                    ENT,          TL_LOWR, XXXXXXX, XXXXXXX, KC_VOLU
    ),

    // LOWER
    [FN1] = LAYOUT(
        XXXXXXX, PIPE,    AT,      HASH,    F4,      F5,             F6,      F7,      F8,      F9,      F10,     BSLS,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LBRC,    RBRC,    XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LCBR,    RCBR,    PK_CPYR,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_VALU, XXXXXXX,
        _______, _______, _______, _______,    _______,                  _______,      _______, RM_SPDU, RM_VALD, RM_SPDD
    ),

    // UPPER
    [FN2] = LAYOUT(
        _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, BSLS,
        ESC,     N1,      N2,      N3,      N4,      N5,             N6,      N7,      N8,      N9,      N0,      XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        LEFT,    DOWN,    UP,      RIGHT,   XXXXXXX, XXXXXXX,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_VALU, XXXXXXX,
        _______, _______, _______, _______,     _______,                  _______,     _______, RM_SPDU, RM_VALD, RM_SPDD
    ),

    // Currently unused, and not accessible
    // [FN3] = LAYOUT(
    //     XXXXXXX, PIPE,    AT,      HASH,    TILD,    EURO,           NOT,     XXXXXXX, XXXXXXX, XXXXXXX, QUOT,    BSLS,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TD_GRV,  XXXXXXX,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LCBR,    RCBR,    XXXXXXX,
    //     _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, MINS,    XXXXXXX, XXXXXXX,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,      XXXXXXX,                 _______,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    // ),

    // ADJUST
    [RST] = LAYOUT(
        QK_BOOT, XXXXXXX, F2,      XXXXXXX, F4,      PK_LOG,         XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        PK_QCLR, AC_TOGG, XXXXXXX, XXXXXXX, PK_SIZE, XXXXXXX,        PK_KLOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, _______, _______,     DB_TOGG,                 DB_TOGG,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};
// clang-format on

static uint32_t read_touch_callback(uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(TOUCH_SCREEN_ENABLE) || !IS_DEFINED(RIGHT_HAND)) {
        return 0;
    }

    uint32_t interval = TOUCH_MS;

    if (IS_DEFINED(XAP_ENABLE)) {
        if (!is_ili9341_pressed()) {
            xap_screen_released(ILI9341_ID);
            return interval;
        }
    }

    // Make a read and send it to Tauri
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch, false);
    if (IS_DEFINED(XAP_ENABLE)) {
        xap_screen_pressed(ILI9341_ID, ili9341_touch_report);
    }

    return interval;
}

void keyboard_post_init_keymap(void) {
    if (IS_DEFINED(QUANTUM_PAINTER_ENABLE) && IS_DEFINED(LEFT_HAND)) {
        qp_set_device_by_name("il91874", il91874);
    }

    if (IS_DEFINED(QUANTUM_PAINTER_ENABLE) && IS_DEFINED(RIGHT_HAND)) {
        qp_set_device_by_name("ili9163", ili9163);
        qp_set_device_by_name("ili9341", ili9341);

        // FIXME:
        // set_uptime_device(ili9341);
        set_logging_device(ili9341);
        // set_heap_stats_device(ili9341);
        set_layer_device(ili9341);

        if (IS_DEFINED(KEYLOG_ENABLE)) {
            set_keylog_device(ili9341);
        }

        if (IS_DEFINED(TOUCH_SCREEN_ENABLE)) {
            defer_exec(MILLISECONDS(10), read_touch_callback, NULL);
        }
    }

#if (COMMUNITY_MODULE_RNG_ENABLE)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
}

void build_info_sync_keymap_callback(void) {
    if (IS_DEFINED(LEFT_HAND) && IS_DEFINED(QUANTUM_PAINTER_ENABLE)) {
        draw_commit(il91874);
        draw_features(il91874);
    }
}

#if defined(COMMUNITY_MODULE_LEDMAP_ENABLE)
// clang-format off
const ledmap_color_t PROGMEM ledmap[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT(
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,    BLACK,         WHITE,    RED,  TRNS, RED,  RED
    ),
    [FN1] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,    CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,
        BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
        ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,    ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,
        WHITE,WHITE,BLACK,TRNS,    BLACK,         BLACK,   RED,  TRNS, WHITE,WHITE
    ),
    [FN2] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS,    TRNS,          TRNS,    TRNS, TRNS, TRNS, TRNS
    ),
    [RST] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS,    TRNS,          TRNS,    TRNS, TRNS, TRNS, TRNS
    ),
};
// clang-format on
#endif

#if defined(COMMUNITY_MODULE_INDICATORS_ENABLE)
const indicator_t PROGMEM indicators[] = {
    LAYER_INDICATOR(RST, RGB_OFF),

    // QMK keycodes
    KEYCODE_IN_LAYER_INDICATOR(QK_BOOT, RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(QK_RBT, RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(EE_CLR, RST, RGB_RED),
    KEYCODE_IN_LAYER_INDICATOR(DB_TOGG, RST, RGB_RED),
    // KEYCODE_IN_LAYER_INDICATOR(AC_DICT, RST, RGB_RED),

    // custom keycodes
    CUSTOM_KEYCODE_IN_LAYER_INDICATOR(RST, RGB_BLUE),
};
#endif

bool rgb_matrix_indicators_advanced_keymap(__unused uint8_t led_min, __unused uint8_t led_max) {
#if defined(COMMUNITY_MODULE_MICROPYTHON_ENABLE)
#    include "py/rgb_effect.c"
    mp_embed_exec_str(rgb_effect);
#endif

    return true;
}

const char *log_time(void) {
    static char buff[15];

    div_t secs = div((int)timer_read32(), SECONDS(1));

    string_t str = str_from_buffer(buff);
    str_printf(&str, "%d.%ds", secs.quot, secs.rem);

    return buff;
}
