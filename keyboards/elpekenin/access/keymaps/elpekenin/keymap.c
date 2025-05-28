// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdlib.h>

#include "elpekenin/build_info.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/qp/tasks/computer_stats.h"
#include "elpekenin/qp/tasks/github_notifications.h"
#include "elpekenin/qp/tasks/heap_stats.h"
#include "elpekenin/qp/tasks/keylog.h"
#include "elpekenin/qp/tasks/layer.h"
#include "elpekenin/qp/tasks/logging.h"
#include "elpekenin/qp/tasks/uptime.h"
#include "elpekenin/signatures.h"
#include "elpekenin/time.h"
#include "elpekenin/xap.h"

#if CM_ENABLED(INDICATORS)
#    include "elpekenin/indicators.h"
#endif

#if CM_ENABLED(LEDMAP)
#    include "elpekenin/ledmap.h"
#endif

#if CM_ENABLED(MICROPYTHON)
#    include "port/micropython_embed.h"
#endif

#if CM_ENABLED(RNG)
#    include <platforms/chibios/drivers/analog.h>

#    include "elpekenin/rng.h"
#endif

#if CM_ENABLED(STRING)
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

static uint32_t read_touch_callback(__unused uint32_t trigger_time, __unused void *cb_arg) {
    if (!IS_ENABLED(TOUCH_SCREEN) || !IS_DEFINED(RIGHT_HAND)) {
        return 0;
    }

    static touch_report_t last = {
        .pressed = false,
    };

    const bool pressed = is_ili9341_pressed();
    if (pressed) {
        const touch_report_t now = get_spi_touch_report(ili9341_touch, false);
        if (IS_ENABLED(XAP)) {
            // FIXME: do not hardcode 0
            xap_screen_pressed(0, now);
        }
        last = now;
    } else {
        // notify about release, if XAP is enabled
        if (last.pressed && IS_ENABLED(XAP)) {
            xap_screen_released(0);
        }

        last.pressed = false;
    }

    return MILLISECONDS(100);
}

static void configure_qp_tasks(void) {
    painter_font_handle_t font = qp_get_font_by_name("fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    const uint16_t x_coord = 10;
    const uint16_t start_y = 10;
    const uint16_t spacing = font->line_height + 1;

    qp_callback_args_t *keylog = get_keylog_args();
    if (keylog != NULL) {
        *keylog = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = x_coord,
            .y      = start_y,
        };
    }

    qp_callback_args_t *uptime = get_uptime_args();
    if (uptime != NULL) {
        *uptime = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = x_coord,
            .y      = start_y + spacing,
        };
    }

    qp_callback_args_t *layer = get_layer_args();
    if (layer != NULL) {
        *layer = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = x_coord,
            .y      = start_y + (2 * spacing),
        };
    }

    qp_callback_args_t *heap_stats = get_heap_stats_args();
    if (heap_stats != NULL) {
        *heap_stats = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = x_coord,
            .y      = start_y + (3 * spacing),
        };
    }

    qp_callback_args_t *logging = get_logging_args();
    if (logging != NULL) {
        *logging = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = ILI9341_WIDTH / 2,
            .y      = start_y,
            .scrolling_args =
                {
                    .delay   = MILLISECONDS(500),
                    .n_chars = 18, // FIXME: compute at runtime
                },
        };
    }

    const uint16_t      graph_size     = 130;
    qp_callback_args_t *computer_stats = get_computer_stats_args();
    if (computer_stats != NULL) {
        *computer_stats = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = x_coord,
            .y      = start_y + (4 * spacing),
            .extra  = (void *)(uintptr_t)graph_size,
        };
    }

    qp_callback_args_t *github_notifications = get_github_notifications_args();
    if (github_notifications != NULL) {
        painter_image_handle_t github = qp_get_image_by_name("github");
        const uint16_t         offset = (github == NULL) ? 50 : github->width;

        *github_notifications = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = (ILI9341_WIDTH / 2) - offset,
            .y      = start_y,
        };
    }
}

void keyboard_post_init_keymap(void) {
    if (IS_ENABLED(QUANTUM_PAINTER) && IS_DEFINED(LEFT_HAND)) {
        qp_set_device_by_name("il91874", il91874);
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && IS_DEFINED(RIGHT_HAND)) {
        qp_set_device_by_name("ili9163", ili9163);
        qp_set_device_by_name("ili9341", ili9341);

        configure_qp_tasks();
    }

    if (IS_ENABLED(TOUCH_SCREEN) && IS_DEFINED(RIGHT_HAND)) {
        defer_exec(MILLISECONDS(10), read_touch_callback, NULL);
    }

#if CM_ENABLED(RNG)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
}

void build_info_sync_keymap_callback(void) {
    if (!IS_DEFINED(LEFT_HAND) || !IS_ENABLED(QUANTUM_PAINTER)) {
        return;
    }

    // FIXME: where to put this signature
    void draw_features(painter_device_t);

    draw_features(il91874);

    // commit
    painter_font_handle_t font = qp_get_font_by_name("fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    uint16_t width  = qp_get_width(il91874);
    uint16_t height = qp_get_height(il91874);

    const char *commit     = get_build_info().commit;
    int16_t     hash_width = qp_textwidth(font, commit);

    uint16_t x = width - hash_width;
    uint16_t y = height - font->line_height;

    qp_drawtext_recolor(il91874, x, y, font, commit, HSV_RED, HSV_WHITE);
}

#if CM_ENABLED(LEDMAP)
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

#if CM_ENABLED(INDICATORS)
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
#if CM_ENABLED(MICROPYTHON)
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
