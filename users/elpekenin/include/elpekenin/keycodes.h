// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// clang-format off

#pragma once

#include <quantum/quantum_keycodes.h>

#include <quantum/keymap_extras/keymap_spanish.h>

// aliases
#define _______ KC_TRNS
#define R_SPC   LT(_FN3, KC_SPC)

#define KC(x) ES_##x /* just so that adapting this code to other languages is easy */

// these do not have ES_*
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

#define RABK LSFT(LABK) /* cant use S(), it gets redefined */

#define PIPE KC(PIPE)
#define AT   KC(AT)
#define HASH KC(HASH)
#define LABK KC(LABK)
#define PLUS KC(PLUS)
#define COMM KC(COMM)
#define DOT  KC(DOT)
#define MINS KC(MINS)
#define GRV  KC(GRV)
#define BSLS KC(BSLS)
#define LBRC KC(LBRC)
#define RBRC KC(RBRC)
#define LCBR KC(LCBR)
#define RCBR KC(RCBR)

#define N0 KC(0)
#define N1 KC(1)
#define N2 KC(2)
#define N3 KC(3)
#define N4 KC(4)
#define N5 KC(5)
#define N6 KC(6)
#define N7 KC(7)
#define N8 KC(8)
#define N9 KC(9)

#undef  A
#undef  C
#undef  G
#undef  S
#define A KC(A)
#define B KC(B)
#define C KC(C)
#define D KC(D)
#define E KC(E)
#define F KC(F)
#define G KC(G)
#define H KC(H)
#define I KC(I)
#define J KC(J)
#define K KC(K)
#define L KC(L)
#define M KC(M)
#define N KC(N)
#define O KC(O)
#define P KC(P)
#define Q KC(Q)
#define R KC(R)
#define S KC(S)
#define T KC(T)
#define U KC(U)
#define V KC(V)
#define W KC(W)
#define X KC(X)
#define Y KC(Y)
#define Z KC(Z)

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
