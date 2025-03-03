// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// clang-format off

#pragma once

#include <quantum/quantum_keycodes.h>

#include <quantum/keymap_extras/keymap_spanish.h>

// aliases
#define _______ KC_TRNS
#define R_SPC   LT(_FN3, KC_SPC)

#define CTL KC_LCTL
#define SFT KC_LSFT
#define GUI KC_LGUI
#define ALT KC_LALT

#define ESC  KC_ESC
#define BSPC KC_BSPC
#define TAB  KC_TAB
#define SPC  KC_SPC
#define ENT  KC_ENT
#define DEL  KC_DEL

#undef  UP
#define UP    KC_UP
#define LEFT  KC_LEFT
#define DOWN  KC_DOWN
#define RIGHT KC_RIGHT

#define F1  KC_F1
#define F2  KC_F2
#define F3  KC_F3
#define F4  KC_F4
#define F5  KC_F5
#define F6  KC_F6
#define F7  KC_F7
#define F8  KC_F8
#define F9  KC_F9
#define F10 KC_F10
#define F11 KC_F11
#define F12 KC_F12

#define RABK LSFT(LABK) /* can't use S(), it gets redefined */

#define PIPE ES_PIPE
#define AT   ES_AT
#define HASH ES_HASH
#define LABK ES_LABK
#define PLUS ES_PLUS
#define COMM ES_COMM
#define DOT  ES_DOT
#define MINS ES_MINS
#define GRV  ES_GRV
#define BSLS ES_BSLS
#define LBRC ES_LBRC
#define RBRC ES_RBRC
#define LCBR ES_LCBR
#define RCBR ES_RCBR

#define N0 ES_0
#define N1 ES_1
#define N2 ES_2
#define N3 ES_3
#define N4 ES_4
#define N5 ES_5
#define N6 ES_6
#define N7 ES_7
#define N8 ES_8
#define N9 ES_9

#undef  A
#undef  C
#undef  G
#undef  S
#define A ES_A
#define B ES_B
#define C ES_C
#define D ES_D
#define E ES_E
#define F ES_F
#define G ES_G
#define H ES_H
#define I ES_I
#define J ES_J
#define K ES_K
#define L ES_L
#define M ES_M
#define N ES_N
#define O ES_O
#define P ES_P
#define Q ES_Q
#define R ES_R
#define S ES_S
#define T ES_T
#define U ES_U
#define V ES_V
#define W ES_W
#define X ES_X
#define Y ES_Y
#define Z ES_Z

// note: these are always defined, but may not do anything based on features enabled
enum userspace_keycodes {
    __CUSTOM_KEYCODES_START = QK_USER,

    PK_CPYR, // copyright header

    // QP
    PK_QCLR, // clear logging (print)
    PK_KLOG, // toggle keylogger

    // Unicode
    PK_UCIS, // starts UCIS mode

    // Logging
    PK_LOG, // cycle overall logging level for general features (UNKNOWN)

    // Games
    PK_GAME, // start a new game

    // Crash
    PK_CRSH, // crash the firmware (testing)
    PK_PCSH, // print crash call stack

    PK_SIZE, // print used flash

    // First value that can be used on a keymap, keep last
    QK_KEYMAP
};

#if defined(TAP_DANCE_ENABLE)
enum userspace_tapdances {
    _TD_NTIL,
    _TD_Z,
    _TD_GRV,
    _TD_SPC,
};

#    define TD_NTIL TD(_TD_NTIL)
#    define TD_Z    TD(_TD_Z)
#    define TD_GRV  TD(_TD_GRV)
#    define TD_SPC  TD(_TD_SPC)
#else
#    define TD_NTIL ES_NTIL
#    define TD_Z    KC_Z
#    define TD_GRV  ES_GRV
#    define TD_SPC  KC_SPC
#endif // TAP_DANCE_ENABLE
