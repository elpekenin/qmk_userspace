// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/xap.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"

#include "generated/keycode_str.h"

#if defined(TOUCH_SCREEN_ENABLE)
void xap_screen_pressed(uint8_t screen_id, touch_report_t report) {
    screen_pressed_msg_t msg = {.msg_id = _SCREEN_PRESSED, .screen_id = screen_id, .x = report.x, .y = report.y};

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_screen_released(uint8_t screen_id) {
    screen_released_msg_t msg = {.msg_id = _SCREEN_RELEASED, .screen_id = screen_id};

    xap_broadcast_user(&msg, sizeof(msg));
}
#endif

void xap_layer(layer_state_t state) {
    layer_change_msg_t msg = {.msg_id = _LAYER_CHANGE, .layer = get_highest_layer(state)};

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_keyevent(uint16_t keycode, keyrecord_t *record) {
    keyevent_msg_t msg = {
        .base =
            {
                .msg_id  = _KEYEVENT,
                .keycode = keycode,
                .pressed = record->event.pressed,
                .layer   = get_highest_layer(layer_state),
                .row     = record->event.key.row,
                .col     = record->event.key.col,
                .mods    = get_mods(),
            },
    };

    const char *name = get_keycode_name(keycode);
    name             = (name == NULL) ? "XXX" : name;
    strncpy(msg.str, name, sizeof(msg.str));

    // variable size                          '\0' ~~~v
    size_t len = sizeof(msg.base) + strlen(msg.str) + 1;
    //                     ^~~~ keyevent_msg_t.base is not valid C
    xap_broadcast_user(&msg, len);
}

void xap_shutdown(bool jump_to_bootloader) {
    if (!is_keyboard_master()) {
        return;
    }

    shutdown_msg_t msg = {
        .msg_id     = _SHUTDOWN,
        .bootloader = jump_to_bootloader,
    };

    xap_broadcast_user(&msg, sizeof(msg));
}
