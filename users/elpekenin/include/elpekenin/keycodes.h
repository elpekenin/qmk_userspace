// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// clang-format off

#pragma once

#include <quantum/keycodes.h>
#include <quantum/quantum_keycodes.h>
#include <quantum/keymap_extras/keymap_spanish.h>

// note: these are always defined, but may not do anything based on features enabled
enum userspace_keycodes {
    CUSTOM_KEYCODES_START = QK_USER,

    PK_CPYR, // copyright header

    // QP
    PK_QCLR, // clear logging buffer
    PK_KLOG, // toggle keylogger

    // Logging
    PK_LOG, // cycle logging level
    PK_SIZE, // print used flash

    PK_CONF, // print autoconf settings
    PK_ID, // print build id

    // !! remember to update KEYCODE_STRING_NAMES_USER

    // First value that can be used on a keymap, keep last
    QK_KEYMAP
};

enum userspace_tapdances {
    TD_ID_NTIL,
    TD_ID_Z,
    TD_ID_GRV,
    TD_ID_SPC,
};

#define TD_NTIL TD(TD_ID_NTIL)
#define TD_Z    TD(TD_ID_Z)
#define TD_GRV  TD(TD_ID_GRV)
#define TD_SPC  TD(TD_ID_SPC)

// aliases
#define _______ KC_TRNS
#define R_SPC   LT(FN3, KC_SPC)
