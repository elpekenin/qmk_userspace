// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#if defined(TRI_LAYER_ENABLE)
#    include "tri_layer.h"
#endif

#include "elpekenin/layers.h"
#include "elpekenin/signatures.h"
#include "elpekenin/utils/sections.h"

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

#if defined(TRI_LAYER_ENABLE)
static void tri_layer_init(void) {
    set_tri_layer_lower_layer(_FN1);
    set_tri_layer_upper_layer(_FN2);
    set_tri_layer_adjust_layer(_RST);
}
PEKE_PRE_INIT(tri_layer_init, INIT_TRI_LAYER);
#endif

const char *get_layer_name(layer_names_t layer) {
    switch (layer) {
        case _QWERTY:
            return " QWERTY ";

        case _FN1:
            return " F-KEYS ";

        case _FN2:
            return " NUMPAD ";

        case _FN3:
            return "SYMBOLS ";

        case _RST:
            return "  UTILS ";

        default:
            return "????????";
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    state = layer_state_set_keymap(state);

#if defined(XAP_ENABLE)
    xap_layer(state);
#endif

    return state;
}
