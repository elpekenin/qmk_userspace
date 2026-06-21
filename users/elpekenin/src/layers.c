// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/layers.h"

#include "elpekenin/events.h"
#include "elpekenin/m5.h"
#include "elpekenin/signatures.h"
#include "tri_layer.h"

const char *get_layer_name(uint8_t layer) {
    switch ((layer_t)layer) {
        case QWERTY:
            return "QWERTY";

        case FN1:
            return "F-KEYS";

        case FN2:
            return "NUMPAD";

        case FN3:
            return "SYMBOLS";

        case RST:
            return "UTILS";

        default:
            return "???";
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    state = layer_state_set_keymap(state);

    const layer_change_msg_t msg = make_layer_change(state);

    if (IS_ENABLED(XAP)) {
        xap_broadcast_user(&msg, sizeof(msg));
    }

    if (IS_ENABLED(M5)) {
        m5_send(&msg, sizeof(msg));
    }

    return state;
}
