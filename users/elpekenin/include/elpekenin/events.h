// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdbool.h>
#include <stdint.h>

#if __has_include("quantum.h")
#    define _IS_QMK 1
#    include <quantum/quantum.h>

#    include "elpekenin/touch.h"
#else
#    define _IS_QMK 0
#endif

#define MAX_PAYLOAD (XAP_EPSIZE - sizeof(xap_broadcast_header_t))

/**
 * Identifier for each type of event.
 */
typedef enum {
    SCREEN_PRESSED,
    SCREEN_RELEASED,
    LAYER_CHANGE,
    KEYEVENT,
    SHUTDOWN,
    MIC_START,
    MIC_END,
    N_EVENTS,
} event_id_t;
STATIC_ASSERT(~(event_id_t)0 <= UINT8_MAX, "event_id_t expected to be 8bit");

/**
 * Inform about a screen press event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    event_id_t msg_id;

    /**
     * Identify the screen.
     */
    uint8_t screen_id;

    /**
     * X-coord of the touchpoint.
     */
    uint16_t x;

    /**
     * Y-coord of the touchpoint.
     */
    uint16_t y;
} screen_pressed_msg_t;

/**
 * Information about a screen release event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    event_id_t msg_id;

    /**
     * Identify the screen.
     */
    uint8_t screen_id;
} screen_released_msg_t;

/**
 * Information about a layer change event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    event_id_t msg_id;

    /**
     * Layer state.
     */
    uint8_t layer;
} layer_change_msg_t;
STATIC_ASSERT(sizeof(layer_state_t) == sizeof(uint8_t), "Client code expects layer to be u8");

/**
 * Information about a key event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    event_id_t msg_id;

    /**
     * Keycode involved.
     */
    uint16_t keycode;

    /**
     * Whether it got pressed or released.
     */
    bool pressed;

    /**
     * Highest layer currently enabled.
     */
    uint8_t layer;

    /**
     * Electrical row of the key.
     */
    uint8_t row;

    /**
     * Electrical column of the key.
     */
    uint8_t col;

    /**
     * Current modifiers.
     */
    uint8_t mods;

    /**
     * String representation of the keycode.
     */
    char str[32];

    /**
     * Ensure we have a null terminator byte.
     */
    uint8_t null;
} keyevent_msg_t;

/**
 * Information about shutdown event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    event_id_t msg_id;

    /**
     * Whether MCU is rebooting or jumping to bootloader.
     */
    bool bootloader;
} shutdown_msg_t;

/**
 * Enable/disable microphone.
 */
typedef struct PACKED {
    event_id_t msg_id;
} mic_msg_t;

#if defined(_IS_QMK)
screen_pressed_msg_t  make_screen_pressed(uint8_t screen_id, touch_report_t report);
screen_released_msg_t make_screen_released(uint8_t screen_id);
layer_change_msg_t    make_layer_change(layer_state_t state);
keyevent_msg_t        make_key_event(uint16_t keycode, keyrecord_t *record);
shutdown_msg_t        make_shutdown(bool bootloader);
mic_msg_t             make_mic(bool enable);
#endif
