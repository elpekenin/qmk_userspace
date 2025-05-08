// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Utilities on top of XAP.
 *
 * .. seealso::
 *    QMK's `documentation <https://github.com/qmk/qmk_firmware/blob/xap/docs/xap_0.3.0.md>`_ about XAP.
 *
 * .. caution::
 *   XAP is still work in progress.
 *
 *   `Pull Request <https://github.com/qmk/qmk_firmware/pull/13733>`_ for the feature.
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>
#include <quantum/util.h>
#include <tmk_core/protocol/usb_descriptor.h> // XAP_EPSIZE

#include "elpekenin/touch.h"

#define MAX_PAYLOAD (XAP_EPSIZE - sizeof(xap_broadcast_header_t))

/**
 * Identifier for each type of message.
 */
typedef enum {
    SCREEN_PRESSED,
    SCREEN_RELEASED,
    LAYER_CHANGE,
    KEYEVENT,
    SHUTDOWN,
    N_XAP_MSGS,
} xap_msg_id_t;
_Static_assert(N_XAP_MSGS <= UINT8_MAX, "too many identifiers for a u8");

/**
 * Inform about a screen press event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    xap_msg_id_t msg_id;

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
 * Send a message to PC's client about a screen press event.
 */
void xap_screen_pressed(uint8_t screen_id, touch_report_t report);

/**
 * Information about a screen release event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    xap_msg_id_t msg_id;

    /**
     * Identify the screen.
     */
    uint8_t screen_id;
} screen_released_msg_t;

/**
 * Send a message to PC's client about a screen release event.
 */
void xap_screen_released(uint8_t screen_id);

/**
 * Information about a layer change event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    xap_msg_id_t msg_id;

    /**
     * Layer state.
     */
    layer_state_t layer;
} layer_change_msg_t;
_Static_assert(sizeof(layer_state_t) == sizeof(uint8_t), "Client code expects layer to be u8");

/**
 * Send a message to PC's client about a layer change event.
 */
void xap_layer(layer_state_t state);

/**
 * Information about a key event.
 */
typedef struct PACKED {
    /**
     * Internal type used to hold event's information.
     *
     * Defined an inner struct instead of plain attributes to compute space left for the string.
     */
    struct PACKED base {
        /**
         * Identify this message.
         */
        xap_msg_id_t msg_id;

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
    } base;

// compat: xap_broadcast_header_t does not exist if XAP is not enabled
//         because it's generated code, which would make MAX_PAYLOAD error
#if defined(XAP_ENABLE) || defined(__SPHINX__)
    /**
     * String representation of the keycode.
     */
    char str[MAX_PAYLOAD - sizeof(struct base) - 1];
#endif

    /**
     * Ensure we have a null terminator byte.
     */
    uint8_t null;
} keyevent_msg_t;
#if defined(XAP_ENABLE)
_Static_assert(sizeof(keyevent_msg_t) == MAX_PAYLOAD, "wrong size for keyevent_msg_t");
#endif

/**
 * Send a message to PC's client about a key event.
 */
void xap_keyevent(uint16_t keycode, keyrecord_t *record);

/**
 * Information about shutdown event.
 */
typedef struct PACKED {
    /**
     * Identify this message.
     */
    xap_msg_id_t msg_id;

    /**
     * Whether MCU is rebooting or jumping to bootloader.
     */
    bool bootloader;
} shutdown_msg_t;

/**
 * Send a message to PC's client about a shutdown event.
 */
void xap_shutdown(bool bootloader);
