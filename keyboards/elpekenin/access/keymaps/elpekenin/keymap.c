// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <platforms/chibios/drivers/analog.h>
#include <quantum/color.h>
#include <quantum/compiler_support.h>
#include <quantum/split_common/split_util.h>
#include <stdlib.h>

#include "elpekenin/autoconf_rt.h"
#include "elpekenin/eeprom.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/signatures.h"
#include "elpekenin/xap.h"
#include "generated/qp_resources.h" // access to fonts/images

#if CM_ENABLED(INDICATORS)
#    include "elpekenin/indicators.h"
#endif

#if CM_ENABLED(KEYLOG)
#    include "elpekenin/keylog.h"
#endif

#if CM_ENABLED(LEDMAP)
#    include "elpekenin/ledmap.h"
#endif

#if CM_ENABLED(MICROPYTHON)
#    include "port/micropython_embed.h"
// code
#    include "py/rgb_effect.c"
#    include "py/version.c"
#endif

#if CM_ENABLED(RNG)
#    include "elpekenin/rng.h"
#endif

#if CM_ENABLED(UI)
#    include "elpekenin/ui.h"
extern ui_node_t root; // on ./ui.c
#endif

STATIC_ASSERT(CM_ENABLED(BUILD_ID), "Must enable 'elpekenin/build_id'");
#include "elpekenin/build_id.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
#include "elpekenin/logging.h"

STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");
#include "elpekenin/string.h"

enum keymap_keycodes {
    PK_PY = QK_KEYMAP, // print QMK version from MicroPython
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT(
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,          KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_BSPC,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,          KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     ES_PLUS,
        TD_ANGL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,          KC_H,     KC_J,     KC_K,     KC_L,     TD_NTIL,  XXXXXXX,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,          KC_N,     KC_M,     KC_COMM,  ES_DOT,   ES_MINS,  ES_GRV,
        KC_LCTL,  KC_LGUI,  KC_LALT,  TL_UPPR,      TD_SPC,                    KC_ENT,       TL_LOWR,  XXXXXXX,  XXXXXXX,  KC_VOLU
    ),

    // LOWER
    [FN1] = LAYOUT(
        XXXXXXX,  ES_PIPE,  ES_AT,    ES_HASH,  ES_TILD,  ES_EURO,       ES_NOT,   XXXXXXX,  XXXXXXX,  XXXXXXX,  ES_QUOT,  ES_BSLS,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  ES_LBRC,  ES_RBRC,  XXXXXXX,  PK_CPYR,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  ES_LCBR,  ES_RCBR,  RM_VALU,  XXXXXXX,
        _______,  _______,  _______,  _______,      _______,                 _______,        _______,  RM_SPDU,  RM_VALD,  RM_SPDD
    ),

    // UPPER
    [FN2] = LAYOUT(
        _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,         KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   ES_BSLS,
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,          KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       KC_LEFT,  KC_DOWN,  KC_UP,    KC_RIGHT, XXXXXXX,  XXXXXXX,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,       XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  RM_VALU,  XXXXXXX,
        _______,  _______,  _______,  _______,      _______,                 _______,        _______,  RM_SPDU,  RM_VALD,  RM_SPDD
    ),

    // ADJUST
    [RST] = LAYOUT(
        QK_BOOT,  XXXXXXX,  KC_F2,    XXXXXXX,  KC_F4,   PK_LOG,         PK_ID,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        PK_QCLR,  AC_TOGG,  XXXXXXX,  XXXXXXX,  PK_SIZE, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, PK_PY,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX,  XXXXXXX,  _______,  _______,      DB_TOGG,                 PK_CONF,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};
// clang-format on

//
// helper functions
//

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

    return 100;
}

static void render_autoconf(void) {
    painter_font_handle_t font = qp_load_font_mem(font_ubuntu);
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

        // limit name to 20 chars, making sure text will fit horizontally
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

        if ((pos.y + font->line_height) >= qp_get_height(il91874)) {
            if (pos.shifted) {
                logging(LOG_ERROR, "No space left in screen");
                break;
            }

            pos.x = qp_get_width(il91874) / 2;
            pos.y = 0;

            pos.shifted = true;
        }
    }

    qp_close_font(font);
}

#if CM_ENABLED(LEDMAP)
// clang-format off
const ledmap_color_t PROGMEM ledmap[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS,    TRNS,          TRNS,    TRNS, TRNS, TRNS, TRNS
    ),
    [FN1] = LAYOUT(
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
        TRNS, TRNS, TRNS, TRNS,    TRNS,          TRNS,    TRNS, TRNS, TRNS, TRNS
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
    NUM_LOCK_INDICATOR(HUE(HUE_RED)),

    LAYER_INDICATOR(RST, RGB_COLOR(RGB_OFF)),

    // QMK keycodes
    KEYCODE_IN_LAYER_INDICATOR(QK_BOOT, RST, HUE(HUE_RED)),
    KEYCODE_IN_LAYER_INDICATOR(QK_RBT, RST, HUE(HUE_RED)),
    KEYCODE_IN_LAYER_INDICATOR(EE_CLR, RST, HUE(HUE_RED)),
    KEYCODE_IN_LAYER_INDICATOR(DB_TOGG, RST, HUE(HUE_RED)),
    // KEYCODE_IN_LAYER_INDICATOR(AC_DICT, RST, RGB_RED),

    // custom keycodes
    CUSTOM_KEYCODE_IN_LAYER_INDICATOR(RST, HUE(HUE_BLUE)),
};
#endif

const char *log_time(void) {
    static char buff[15];

    div_t secs = div((int)timer_read32(), 1000);

    string_t str = str_from_buffer(buff);
    str_printf(&str, "%d.%ds", secs.quot, secs.rem);

    return buff;
}

//
// QMK hooks
//

void keyboard_post_init_keymap(void) {
    if (IS_ENABLED(QUANTUM_PAINTER) && is_keyboard_left()) {
        set_device_by_name("il91874", il91874);

        u128       running_build_id;
        const bool err = get_build_id(&running_build_id) < 0;

        user_data_t eeprom = {0};
        eeconfig_read_user_datablock_field(eeprom, build_id);

        const bool new_fw = !err && memcmp(&running_build_id, &eeprom.build_id, sizeof(u128)) != 0;
        // NOTE: sub-optimal because screen won't be drawn immediately
        //       it is flushed 3 minutes after boot, to prevent damaging it
        //       this means that losing power during that time will cause no redraw but storing new id
        //       which in turn means next power cycle won't draw as eeprom value matches
        if (new_fw) {
            eeprom.build_id = running_build_id;
            eeconfig_update_user_datablock_field(eeprom, build_id);
        }

        // FIXME: AUTOCONF_FW_CHECK is a hack to fix screen getting wiped (removes text)
        //        find out the root cause of that, and remove this workaround
        const bool skip_draw = IS_DEFINED(AUTOCONF_FW_CHECK) && new_fw;
        // only draw when a new firmware is flashed
        // this may still be a "useless" redraw (no settings changed)
        // but is already much better than drawing on every power cycle
        if (!skip_draw) {
            render_autoconf();
        }
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && !is_keyboard_left()) {
        set_device_by_name("ili9163", ili9163);
        set_device_by_name("ili9341", ili9341);

#if CM_ENABLED(UI)
        ui_init(&root, qp_get_width(ili9341), qp_get_height(ili9341));
#endif
    }

    if (IS_ENABLED(TOUCH_SCREEN) && !is_keyboard_left()) {
        defer_exec(10, read_touch_callback, NULL);
    }

    // NOTE: analog macro is not provided by QMK, but custom Kconfig
#if CM_ENABLED(RNG) && IS_DEFINED(ANALOG_DRIVER_REQUIRED)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
}

bool rgb_matrix_indicators_advanced_keymap(__unused uint8_t led_min, __unused uint8_t led_max) {
#if CM_ENABLED(MICROPYTHON)
    // mp_embed_exec_str(rgb_effect);
#endif

    return true;
}

void housekeeping_task_keymap(void) {
    if (is_keyboard_left()) {
        return;
    }

#if CM_ENABLED(UI)
    ui_render(&root, ili9341);
#endif
}

bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case PK_PY:
            if (record->event.pressed) {
#if CM_ENABLED(MICROPYTHON)
                mp_embed_exec_str(version);
#endif
            }
            return false;

        default:
            return true;
    }
}
