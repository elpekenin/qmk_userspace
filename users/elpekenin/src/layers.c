// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/layers.h"

#include "elpekenin/signatures.h"
#include "elpekenin/xap.h"
#include "tri_layer.h"

const char *get_layer_name(layer_t layer) {
    switch (layer) {
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

    if (IS_ENABLED(XAP)) {
        xap_layer(state);
    }

    return state;
}
