// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// This file gets included by QMK's introspection, add here any feature code
// that is required by it. (combos, tapdance, dip switch, etc)

#include <quantum/process_keycode/process_key_override.h>
#include <quantum/process_keycode/process_tap_dance.h>
#include <quantum/quantum.h>

#include "elpekenin/keycodes.h"
#include "elpekenin/time.h"

void td_ntil_finished(tap_dance_state_t *state, __unused void *user_data) {
    switch (state->count) {
        case 1:
            tap_code16(ES_ACUT); // ´
            break;

        case 2:
            tap_code16(ES_NTIL); // ñ
            break;

        default:
            break;
    }
}

void td_z_finished(tap_dance_state_t *state, __unused void *user_data) {
    switch (state->count) {
        case 1:
            register_code16(KC_Z); // z
            break;

        case 2:
            tap_code16(ES_LABK); // <
            break;

        case 3:
            tap_code16(ES_RABK); // >
            break;

        default:
            break;
    }
}

void td_z_reset(tap_dance_state_t *state, __unused void *user_data) {
    switch (state->count) {
        case 1:
            unregister_code16(KC_Z);
            break;

        default:
            break;
    }
}

void td_spc_each(tap_dance_state_t *state, __unused void *user_data) {
    unregister_code16(KC_SPC);
    register_code16(KC_SPC);

    if ((state->count % 2) == 0) {
        unregister_code16(KC_SPC);
        tap_code16(KC_SPC);
        register_code16(KC_SPC);
    }
}

void td_spc_reset(__unused tap_dance_state_t *state, __unused void *user_data) {
    unregister_code16(KC_SPC);
}

void td_grv_finished(tap_dance_state_t *state, __unused void *user_data) {
    if (state->count == 1) {
        tap_code16(ES_GRV);
        return;
    }

    // TODO: Change tap-hold decision so this does not work funny
    send_string_with_delay("``` " SS_LSFT("\n\n") "``` ", MILLISECONDS(10));
    tap_code16(KC_UP);
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_ID_NTIL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_ntil_finished, NULL),
    [TD_ID_Z]    = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_z_finished, td_z_reset),
    [TD_ID_SPC]  = ACTION_TAP_DANCE_FN_ADVANCED(td_spc_each, NULL, td_spc_reset),
    [TD_ID_GRV]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_grv_finished, NULL),
};

const key_override_t delete_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_BSPC, KC_DEL);
const key_override_t volume_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_VOLU, KC_VOLD);
const key_override_t alt_f4_key_override = ko_make_basic(MOD_MASK_ALT, KC_4, LALT(KC_F4));

const key_override_t *key_overrides[] = (const key_override_t *[]){
    &delete_key_override,
    &volume_key_override,
    &alt_f4_key_override,
};
