// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <platforms/chibios/drivers/analog.h>
#include <quantum/color.h>
#include <quantum/compiler_support.h>
#include <stdlib.h>

#include "elpekenin/autoconf_rt.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
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
#    include "elpekenin/rng.h"
#endif

STATIC_ASSERT(CM_ENABLED(BUILD_ID), "Must enable 'elpekenin/build_id'");
STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");

#include "elpekenin/build_id.h"
#include "elpekenin/logging.h"
#include "elpekenin/string.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT(
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,          KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_BSPC,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,          KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     ES_PLUS,
        XXXXXXX,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,          KC_H,     KC_J,     KC_K,     KC_L,     TD_NTIL,  XXXXXXX,
        KC_LSFT,  TD_Z,     KC_X,     KC_C,     KC_V,     KC_B,          KC_N,     KC_M,     KC_COMM,  ES_DOT,   ES_MINS,  ES_GRV,
        KC_LCTL,  KC_LGUI,  KC_LALT,  TL_UPPR,      TD_SPC,                    KC_ENT,       TL_LOWR,  XXXXXXX,  XXXXXXX,  KC_VOLU
    ),

    // LOWER
    [FN1] = LAYOUT(
        XXXXXXX,  ES_PIPE,  ES_AT,    ES_HASH,  KC_F4,    KC_F5,         KC_6,     KC_F7,    KC_F8,    KC_F9,    KC_F10,   ES_BSLS,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  ES_LBRC,  ES_RBRC,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  ES_LCBR,  ES_RCBR,  PK_CPYR,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  RM_VALU,  XXXXXXX,
        _______,  _______,  _______,  _______,      _______,                 _______,        _______,  RM_SPDU,  RM_VALD,  RM_SPDD
    ),

    // UPPER
    [FN2] = LAYOUT(
        _______,  _______,  _______,  _______,  _______,  _______,       _______,  _______,  _______,  _______,  _______,  ES_BSLS,
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,          KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       KC_LEFT,  KC_DOWN,  KC_UP,    KC_RIGHT, XXXXXXX,  XXXXXXX,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  RM_VALU,  XXXXXXX,
        _______,  _______,  _______,  _______,      _______,                 _______,        _______,  RM_SPDU,  RM_VALD,  RM_SPDD
    ),

    // ADJUST
    [RST] = LAYOUT(
        QK_BOOT,  XXXXXXX,  KC_F2,    XXXXXXX,  KC_F4,   PK_LOG,         PK_ID,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        PK_QCLR,  AC_TOGG,  XXXXXXX,  XXXXXXX,  PK_SIZE, XXXXXXX,        PK_KLOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX,  XXXXXXX,  _______,  _______,      DB_TOGG,                 PK_CONF,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};
// clang-format on

static uint32_t read_touch_callback(__unused uint32_t trigger_time, __unused void *cb_arg) {
    if (!IS_ENABLED(TOUCH_SCREEN) || is_keyboard_left()) {
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

static void render_autoconf(void) {
    painter_font_handle_t font = qp_get_font_by_name("fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    struct {
        bool     shifted;
        uint16_t x;
        uint16_t y;
    } pos = {0};

    string_t str = str_new(30);

    const autoconf_setting_t *settings = get_autoconf_settings();
    for (size_t i = 0; i < autoconf_settings_count(); ++i) {
        autoconf_setting_t setting = settings[i];

        // most likely a 'enable' flag, skip it
        if (setting.value.type == AUTOCONF_INT && setting.value.integer == 1) {
            continue;
        }

        str_printf(&str, "%.*s=", 20, setting.name);

        switch (setting.value.type) {
            case AUTOCONF_INT:
                str_printf(&str, "%d", setting.value.integer);
                break;

            case AUTOCONF_STRING:
                str_append(&str, setting.value.string);
                break;
        }

        qp_drawtext_recolor(il91874, pos.x, pos.y, font, str.ptr, HSV_BLACK, HSV_WHITE);
        str_reset(&str);

        pos.y += font->line_height;

        if (pos.y >= qp_get_height(il91874)) {
            if (pos.shifted) {
                logging(LOG_ERROR, "No space left in screen");
                return;
            }

            pos.x = qp_get_width(il91874) / 2;
            pos.y = 0;

            pos.shifted = true;
        }
    }
}

static void configure_qp_tasks(void) {
    painter_font_handle_t font = qp_get_font_by_name("fira_code");
    if (font == NULL) {
        logging(LOG_ERROR, "%s: font == NULL", __func__);
        return;
    }

    const uint16_t spacing = font->line_height + 1;

    struct {
        uint16_t x;
        uint16_t y;
    } pos = {
        .x = 10,
        .y = 10,
    };

    qp_callback_args_t *keylog = get_keylog_args();
    if (IS_ENABLED(KEYLOG) && keylog != NULL) {
        *keylog = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = pos.x,
            .y      = pos.y,
        };

        pos.y += spacing;
    }

    qp_callback_args_t *uptime = get_uptime_args();
    if (uptime != NULL) {
        *uptime = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = pos.x,
            .y      = pos.y,
        };

        pos.y += spacing;
    }

    qp_callback_args_t *layer = get_layer_args();
    if (layer != NULL) {
        *layer = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = pos.x,
            .y      = pos.y,
        };

        pos.y += spacing;
    }

    qp_callback_args_t *heap_stats = get_heap_stats_args();
    if (heap_stats != NULL) {
        *heap_stats = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = pos.x,
            .y      = pos.y,
        };

        // NOTE: draws 2 lines, flash & heap
        pos.y += 2 * spacing;
    }

    qp_callback_args_t *computer_stats = get_computer_stats_args();
    if (computer_stats != NULL) {
        const uint16_t graph_size = 130;

        *computer_stats = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = pos.x,
            .y      = pos.y,
            .extra  = (void *)(uintptr_t)graph_size,
        };

        pos.y += graph_size;
    }

    qp_callback_args_t *logging = get_logging_args();
    if (logging != NULL) {
        *logging = (qp_callback_args_t){
            .device = ili9341,
            .font   = font,
            .x      = ILI9341_WIDTH / 2,
            .y      = spacing,
            .scrolling_args =
                {
                    .delay   = MILLISECONDS(500),
                    .n_chars = 18, // FIXME: compute at runtime
                },
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
            .y      = 0,
        };
    }
}

void keyboard_post_init_keymap(void) {
    if (IS_ENABLED(QUANTUM_PAINTER) && is_keyboard_left()) {
        qp_set_device_by_name("il91874", il91874);

        render_autoconf();
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && !is_keyboard_left()) {
        qp_set_device_by_name("ili9163", ili9163);
        qp_set_device_by_name("ili9341", ili9341);

        // left side has no tasks (e-Ink)
        configure_qp_tasks();
    }

    if (IS_ENABLED(TOUCH_SCREEN) && !is_keyboard_left()) {
        defer_exec(MILLISECONDS(10), read_touch_callback, NULL);
    }

    // NOTE: analog macro is not provided by QMK, but custom Kconfig
#if CM_ENABLED(RNG) && IS_DEFINED(ANALOG_DRIVER_REQUIRED)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
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
