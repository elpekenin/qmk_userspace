// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/signatures.h"

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
#else
#    error "This code depends on 'elpekenin/rng' to work"
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/qp/assets.h"
#    include "elpekenin/qp/graphics.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT(
        ESC,     N1,      N2,      N3,      N4,      N5,             N6,      N7,      N8,      N9,      N0,      BSPC,
        TAB,     Q,       W,       E,       R,       T,              Y,       U,       I,       O,       P,       PLUS,
        XXXXXXX, A,       S,       D,       F,       G,              H,       J,       K,       L,       TD_NTIL, XXXXXXX,
        SFT,     TD_Z,    X,       C,       V,       B,              N,       M,       COMM,    DOT,     MINS,    GRV,
        CTL,     GUI,     ALT,     TL_UPPR,   TD_SPC,                    ENT,          TL_LOWR, XXXXXXX, XXXXXXX, KC_VOLU
    ),

    // LOWER
    [_FN1] = LAYOUT(
        XXXXXXX, PIPE,    AT,      HASH,    F4,      F5,             F6,      F7,      F8,      F9,      F10,     BSLS,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LBRC,    RBRC,    XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LCBR,    RCBR,    PK_CPYR,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_VALU, XXXXXXX,
        _______, _______, _______, _______,    _______,                  _______,      _______, RM_SPDU, RM_VALD, RM_SPDD
    ),

    // UPPER
    [_FN2] = LAYOUT(
        _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, BSLS,
        ESC,     N1,      N2,      N3,      N4,      N5,             N6,      N7,      N8,      N9,      N0,      XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        LEFT,    DOWN,    UP,      RIGHT,   XXXXXXX, XXXXXXX,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_VALU, XXXXXXX,
        _______, _______, _______, _______,     _______,                  _______,     _______, RM_SPDU, RM_VALD, RM_SPDD
    ),

    // Currently unused, and not accessible
    // [_FN3] = LAYOUT(
    //     XXXXXXX, PIPE,    AT,      HASH,    TILD,    EURO,           NOT,     XXXXXXX, XXXXXXX, XXXXXXX, QUOT,    BSLS,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TD_GRV,  XXXXXXX,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, LCBR,    RCBR,    XXXXXXX,
    //     _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, MINS,    XXXXXXX, XXXXXXX,
    //     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,      XXXXXXX,                 _______,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    // ),

    // ADJUST
    [_RST] = LAYOUT(
        QK_BOOT, XXXXXXX, F2,      XXXXXXX, F4,      PK_LOG,         XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        PK_QCLR, AC_TOGG, XXXXXXX, XXXXXXX, PK_SIZE, XXXXXXX,        PK_KLOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
     // XXXXXXX, XXXXXXX, _______, XXXXXXX,     DB_TOGG,                 DB_TOGG,      _______, XXXXXXX, XXXXXXX, AC_DICT
        XXXXXXX, XXXXXXX, _______, _______,     DB_TOGG,                 DB_TOGG,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};
// clang-format on

#if defined(QUANTUM_PAINTER_ENABLE) && defined(TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
static uint32_t read_touch_callback(uint32_t trigger_time, void *cb_arg) {
    uint32_t interval = TOUCH_MS;

#    if defined(XAP_ENABLE)
    if (!is_ili9341_pressed()) {
        xap_screen_released(ILI9341_ID);
        return interval;
    }
#    endif

    // Make a read and send it to Tauri
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch, false);

#    if defined(XAP_ENABLE)
    xap_screen_pressed(ILI9341_ID, ili9341_touch_report);
#    else
    (void)ili9341_touch_report;
#    endif

    return interval;
}
#endif

void keyboard_post_init_keymap(void) {
#if defined(QUANTUM_PAINTER_ENABLE)
#    if IS_LEFT_HAND
    qp_set_device_by_name("il91874", il91874);
#    endif

#    if IS_RIGHT_HAND
    qp_set_device_by_name("ili9163", ili9163);
    qp_set_device_by_name("ili9341", ili9341);

    // set_uptime_device(ili9341);
    set_logging_device(ili9341);
    // set_heap_stats_device(ili9341);
    set_layer_device(ili9341);
#        if defined(KEYLOG_ENABLE)
    set_keylog_device(ili9341);
#        endif

#        if defined(TOUCH_SCREEN_ENABLE)
    defer_exec(10, read_touch_callback, NULL);
#        endif

#    endif
#endif

#if defined(COMMUNITY_MODULE_RNG_ENABLE)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
}

void build_info_sync_keymap_callback(void) {
#if IS_LEFT_HAND && defined(QUANTUM_PAINTER_ENABLE)
    draw_commit(il91874);
    draw_features(il91874);
#endif
}

#if defined(COMMUNITY_MODULE_MICROPYTHON_ENABLE)
// clang-format off
static const char program[] =
"import qmk\n"
"\n"
"color = (\n"
"    qmk.rgb.RED if qmk.get_highest_active_layer() == 0\n"
"    else qmk.rgb.GREEN\n"
")\n"
"qmk.rgb.set_color(0, color)\n"
;
// clang-format on
#endif

#if defined(COMMUNITY_MODULE_LEDMAP_ENABLE)
// clang-format off
const ledmap_color_t PROGMEM ledmap[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT(
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
        RED,  RED,  RED,  RED,    BLACK,         WHITE,    RED,  TRNS, RED,  RED
    ),
    [_FN1] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,    CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,
        BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
        ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,    ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,
        WHITE,WHITE,BLACK,TRNS,    BLACK,         BLACK,   RED,  TRNS, WHITE,WHITE
    ),
    [_FN2] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS,    TRNS,          TRNS,    TRNS, TRNS, TRNS, TRNS
    ),
    [_RST] = LAYOUT(
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
#endif

bool rgb_matrix_indicators_advanced_keymap(uint8_t led_min, uint8_t led_max) {
#if defined(COMMUNITY_MODULE_LEDMAP_ENABLE)
    draw_ledmap(led_min, led_max);
#endif

#if defined(COMMUNITY_MODULE_INDICATORS_ENABLE)
    draw_indicators(led_min, led_max);
#endif

#if defined(COMMUNITY_MODULE_MICROPYTHON_ENABLE)
    mp_embed_exec_str(program);
#endif

    return true;
}

const char *log_time(void) {
    static char buff[15];

    div_t secs = div(timer_read32(), 1000);
    snprintf(buff, sizeof(buff), "%d.%ds", secs.quot, secs.rem);

    return buff;
}

void housekeeping_task_keymap(void) {
    static uint8_t cnt = 0;
    if (++cnt == 0) {
        logging(LOG_INFO, "Alive");
    }
}
