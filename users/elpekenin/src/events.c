// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/events.h"

screen_pressed_msg_t make_screen_pressed(uint8_t screen_id, touch_report_t report) {
    return (screen_pressed_msg_t){
        .msg_id    = SCREEN_PRESSED,
        .screen_id = screen_id,
        .x         = report.x,
        .y         = report.y,
    };
}

screen_released_msg_t make_screen_released(uint8_t screen_id) {
    return (screen_released_msg_t){
        .msg_id    = SCREEN_RELEASED,
        .screen_id = screen_id,
    };
}

layer_change_msg_t make_layer_change(layer_state_t state) {
    return (layer_change_msg_t){
        .msg_id = LAYER_CHANGE,
        .layer  = state,
    };
}

keyevent_msg_t make_key_event(uint16_t keycode, keyrecord_t *record) {
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

    return msg;
}

shutdown_msg_t make_shutdown(bool bootloader) {
    return (shutdown_msg_t){
        .msg_id     = SHUTDOWN,
        .bootloader = bootloader,
    };
}

mic_msg_t make_mic(bool enable) {
    return (mic_msg_t){
        .msg_id = enable ? MIC_START : MIC_END,
    };
}
