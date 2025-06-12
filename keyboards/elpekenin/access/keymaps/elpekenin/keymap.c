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
#include "elpekenin/qp/ui/build_match.h"
#include "elpekenin/qp/ui/computer.h"
#include "elpekenin/qp/ui/github.h"
#include "elpekenin/signatures.h"
#include "elpekenin/xap.h"
#include "generated/qp_resources.h" // access to fonts/images

#if IS_ENABLED(QP_LOG)
#    include "elpekenin/logging/backends/qp.h"
#endif

#if CM_ENABLED(ALLOCATOR)
#    include "elpekenin/allocator.h"
#endif

#if CM_ENABLED(INDICATORS)
#    include "elpekenin/indicators.h"
#endif

#if CM_ENABLED(KEYLOG)
#    include "elpekenin/keylog.h"
#endif

#if CM_ENABLED(LEDMAP)
#    include "elpekenin/ledmap.h"
#endif

#if CM_ENABLED(MEMORY)
#    include "elpekenin/memory.h"
#endif

#if CM_ENABLED(MICROPYTHON)
#    include "port/micropython_embed.h"
#endif

#if CM_ENABLED(RNG)
#    include "elpekenin/rng.h"
#endif

STATIC_ASSERT(CM_ENABLED(BUILD_ID), "Must enable 'elpekenin/build_id'");
#include "elpekenin/build_id.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
#include "elpekenin/logging.h"

STATIC_ASSERT(CM_ENABLED(STRING), "Must enable 'elpekenin/string'");
#include "elpekenin/string.h"

STATIC_ASSERT(CM_ENABLED(UI), "Must enable 'elpekenin/ui'");
#include "elpekenin/ui.h"
#include "elpekenin/ui/layer.h"
#include "elpekenin/ui/rgb.h"
#include "elpekenin/ui/text.h"
#include "elpekenin/ui/uptime.h"

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
        PK_QCLR,  AC_TOGG,  XXXXXXX,  XXXXXXX,  PK_SIZE, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX,  XXXXXXX,  _______,  _______,      DB_TOGG,                 PK_CONF,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};
// clang-format on

//
// user interface
//

// clang really wants to indent things far to the right...

// clang-format off
static github_args_t gh_args = {
    .logo = gfx_github,
};

static ui_node_t id[] = {
    {
        // spacer
        .node_size = UI_ABSOLUTE(15),
    },
    {
        .node_size = UI_FONT(1),
        .init      = build_id_init,
        .render    = build_id_render,
        .args      = &(build_id_args_t){
            .font     = font_fira_code,
            .interval = 500,
        },
    },
    {
        .node_size = UI_FONT(1),
        .init      = build_match_init,
        .render    = build_match_render,
        .args      = &(build_match_args_t){
            .font = font_fira_code,
        },
    },
};

static ui_node_t first_row[] = {
    {
        .node_size = UI_IMAGE(1),
        .init      = github_init,
        .render    = github_render,
        .args      = &gh_args,
    },
#if CM_ENABLED(BUILD_ID)
    {
        // spacer
        .node_size = UI_ABSOLUTE(5),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_VERTICAL,
        .children  = UI_CHILDREN(id),
    }
#endif
};

static ui_node_t left[] = {
    {
        .node_size = UI_IMAGE(1),
        .direction = UI_SPLIT_DIR_HORIZONTAL,
        .children  = UI_CHILDREN(first_row),
        .args      = &gh_args,
    },
    {
        .node_size = UI_FONT(1),
        .init      = uptime_init,
        .render    = uptime_render,
        .args      = &(uptime_args_t){
            .font = font_fira_code,
        },
    },
    {
        .node_size = UI_FONT(1),
        .init      = layer_init,
        .render    = layer_render,
        .args      = &(layer_args_t){
            .font       = font_fira_code,
            .interval   = 100,
            .layer_name = get_layer_name,
        },
    },
#if CM_ENABLED(MEMORY)
    {
        .node_size = UI_FONT(1),
        .init      = flash_init,
        .render    = flash_render,
        .args      = &(flash_args_t){
            .font     = font_fira_code,
            .interval = 500,
        },
    },
#endif

#if CM_ENABLED(ALLOCATOR)
    {
        .node_size = UI_FONT(1),
        .init      = heap_init,
        .render    = heap_render,
        .args      = &(heap_args_t){
            .font     = font_fira_code,
            .interval = 500,
        },
    },
#endif

    {
        .node_size = UI_REMAINING(),
        .init      = computer_init,
        .render    = computer_render,
        .args      = &(computer_args_t){},
    }
};

static const uint8_t *const rgb_font = font_fira_code;
static const uint32_t rgb_interval = 500;

static rgb_args_t rgb_args = {
    .font     = rgb_font,
    .interval = rgb_interval,
};

static ui_node_t rgb_mode[] = {
    {
        .node_size = UI_FONT(1),
        .init      = text_init,
        .render    = text_render,
        .args      = &(text_args_t){
            .font     = rgb_font,
            .interval = rgb_interval,
            .str      = "Mode",
        },
    },
    {
        .node_size = UI_FONT(1),
        .init      = rgb_init,
        .render    = rgb_mode_render,
        .args      = &rgb_args,
    },
};

static ui_node_t rgb_hsv[] = {
    {
        .node_size = UI_FONT(1),
        .init      = text_init,
        .render    = text_render,
        .args      = &(text_args_t){
            .font     = rgb_font,
            .interval = rgb_interval,
            .str      = "HSV",
        },
    },
    {
        .node_size = UI_FONT(1),
        .init      = rgb_init,
        .render    = rgb_hsv_render,
        .args      = &rgb_args,
    },
};

static ui_node_t rgb[] = {
    {
        .node_size = UI_RELATIVE(50),
        .direction = UI_SPLIT_DIR_VERTICAL,
        .children  = UI_CHILDREN(rgb_mode),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_VERTICAL,
        .children  = UI_CHILDREN(rgb_hsv),
    },
};

static ui_node_t right[] = {
#if IS_ENABLED(RGB_MATRIX)
    {
        .node_size = UI_FONT(2),
        .direction = UI_SPLIT_DIR_HORIZONTAL,
        .children  = UI_CHILDREN(rgb),
        .args      = &rgb_args,
    },
#endif

#if CM_ENABLED(KEYLOG)
    {
        .node_size = UI_FONT(1),
        .init      = keylog_init,
        .render    = keylog_render,
        .args      = &(keylog_args_t){
            .font = font_fira_code,
        },
    },
#endif

#if IS_ENABLED(QP_LOG)
    {
        .node_size = UI_REMAINING(),
        .init      = qp_logging_init,
        .render    = qp_logging_render,
        .args      = &(qp_logging_args_t){
            .font = font_fira_code,
        },
    },
#endif
};

static ui_node_t nodes[] = {
    {
        .node_size = UI_RELATIVE(50),
        .direction = UI_SPLIT_DIR_VERTICAL,
        .children  = UI_CHILDREN(left),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_VERTICAL,
        .children  = UI_CHILDREN(right),
    },
};

static ui_node_t root = {
    .direction = UI_SPLIT_DIR_HORIZONTAL,
    .children  = UI_CHILDREN(nodes),
};
// clang-format off

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

    return MILLISECONDS(100);
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

const char *log_time(void) {
    static char buff[15];

    div_t secs = div((int)timer_read32(), SECONDS(1));

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

        const u128 running_build_id = get_build_id();

        user_data_t eeprom = {0};
        eeconfig_read_user_datablock_field(eeprom, build_id);

        // only draw when a new firmware is flashed
        // this may still be a "useless" redraw (no settings changed)
        // but is already much better than drawing on every power cycle
        if (memcmp(&running_build_id, &eeprom.build_id, sizeof(u128)) != 0) {
            // NOTE: this is sub-optimal because screen won't be drawn immediately
            //       it is flushed 3 minutes after boot, to prevent damaging it
            //       this means that losing power during that time will cause no redraw but storing new id
            //       which in turn means next power cycle won't draw as eeprom value matches
            eeprom.build_id = running_build_id;
            eeconfig_update_user_datablock_field(eeprom, build_id);

            render_autoconf();
        }
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && !is_keyboard_left()) {
        set_device_by_name("ili9163", ili9163);
        set_device_by_name("ili9341", ili9341);

        ui_init(&root, qp_get_width(ili9341), qp_get_height(ili9341));
    }

    if (IS_ENABLED(TOUCH_SCREEN) && !is_keyboard_left()) {
        defer_exec(MILLISECONDS(10), read_touch_callback, NULL);
    }

    // NOTE: analog macro is not provided by QMK, but custom Kconfig
#if CM_ENABLED(RNG) && IS_DEFINED(ANALOG_DRIVER_REQUIRED)
    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
#endif
}

bool rgb_matrix_indicators_advanced_keymap(__unused uint8_t led_min, __unused uint8_t led_max) {
#if CM_ENABLED(MICROPYTHON)
#    include "py/rgb_effect.c"
    mp_embed_exec_str(rgb_effect);
#endif

    return true;
}

void housekeeping_task_keymap(void) {
    if (is_keyboard_left()) {
        return;
    }

    ui_render(&root, ili9341);
}
