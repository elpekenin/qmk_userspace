// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/xap.h"

#include <quantum/quantum.h>

static uint32_t xap_last_msg = 0;

uint32_t xap_last_activity_time(void) {
    return xap_last_msg;
}

uint32_t xap_last_activity_elapsed(void) {
    return timer_elapsed32(xap_last_msg);
}

void xap_last_activity_update(void) {
    xap_last_msg = timer_read32();
}

void xap_screen_pressed(uint8_t screen_id, touch_report_t report) {
    const screen_pressed_msg_t msg = {
        .msg_id    = SCREEN_PRESSED,
        .screen_id = screen_id,
        .x         = report.x,
        .y         = report.y,
    };

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_screen_released(uint8_t screen_id) {
    const screen_released_msg_t msg = {
        .msg_id    = SCREEN_RELEASED,
        .screen_id = screen_id,
    };

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_layer(layer_state_t state) {
    const layer_change_msg_t msg = {
        .msg_id = LAYER_CHANGE,
        .layer  = get_highest_layer(state),
    };

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_keyevent(uint16_t keycode, keyrecord_t *record) {
    keyevent_msg_t msg = {
        .msg_id  = KEYEVENT,
        .keycode = keycode,
        .pressed = record->event.pressed,
        .layer   = get_highest_layer(layer_state),
        .row     = record->event.key.row,
        .col     = record->event.key.col,
        .mods    = get_mods(),
    };

    const char *name = get_keycode_string(keycode);
    name             = (name == NULL) ? "XXX" : name;
    strncpy(msg.str, name, sizeof(msg.str));

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_shutdown(bool jump_to_bootloader) {
    if (!is_keyboard_master()) {
        return;
    }

    shutdown_msg_t msg = {
        .msg_id     = SHUTDOWN,
        .bootloader = jump_to_bootloader,
    };

    xap_broadcast_user(&msg, sizeof(msg));
}
