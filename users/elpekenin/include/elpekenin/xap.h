// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>
#include <tmk_core/protocol/usb_descriptor.h> // XAP_EPSIZE

#include "elpekenin/utils/compiler.h"

#if defined(TOUCH_SCREEN_ENABLE)
#    include "elpekenin/touch.h"
#endif

// *** Max string length helper ***

#define MAX_PAYLOAD (XAP_EPSIZE - sizeof(xap_broadcast_header_t))

/* the final type will be
 *
 * struct {
 *     base_type;
 *     string;
 *     byte; // make sure we have a terminator
 * }
 */
#define _MAX_STR_LEN(base_type) (MAX_PAYLOAD - sizeof(base_type) - 1)

// *** Identifiers ***

typedef enum {
    _SCREEN_PRESSED,
    _SCREEN_RELEASED,
    _LAYER_CHANGE,
    _KEYEVENT,
    _SHUTDOWN,
} _xap_msg_id_t;

typedef uint8_t xap_msg_id_t;

// *** Messages ***

#if defined(TOUCH_SCREEN_ENABLE)
typedef struct PACKED {
    xap_msg_id_t msg_id;
    uint8_t      screen_id;
    uint16_t     x;
    uint16_t     y;
} screen_pressed_msg_t;

void xap_screen_pressed(uint8_t screen_id, touch_report_t report);

typedef struct PACKED {
    xap_msg_id_t msg_id;
    uint8_t      screen_id;
} screen_released_msg_t;

void xap_screen_released(uint8_t screen_id);
#endif

typedef struct PACKED {
    xap_msg_id_t msg_id;
    uint8_t      layer;
} layer_change_msg_t;

void xap_layer(layer_state_t state);

typedef struct PACKED {
    xap_msg_id_t msg_id;
    uint16_t     keycode;
    bool         pressed;
    uint8_t      layer;
    uint8_t      row;
    uint8_t      col;
    uint8_t      mods;
} _keyevent_msg_t;

typedef struct PACKED {
    _keyevent_msg_t base;
    char            str[_MAX_STR_LEN(_keyevent_msg_t)];
    uint8_t         null;
} keyevent_msg_t;
_Static_assert(sizeof(keyevent_msg_t) == MAX_PAYLOAD, "wrong size for keyevent_msg_t");

NON_NULL(2) READ_ONLY(2) void xap_keyevent(uint16_t keycode, keyrecord_t *record);

typedef struct PACKED {
    xap_msg_id_t msg_id;
    bool         bootloader;
} shutdown_msg_t;
