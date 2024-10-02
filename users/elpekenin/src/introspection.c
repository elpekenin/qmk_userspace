// This file gets included by QMK's introspection, add here any feature code
// that is required by it. (combos, tapdance, dip switch, etc)

#include <quantum/quantum.h>

#include "elpekenin/keycodes.h"

#if defined(TAP_DANCE_ENABLE)
void td_ntil_finished(tap_dance_state_t *state, void *build_info) {
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

void td_z_finished(tap_dance_state_t *state, void *build_info) {
    switch (state->count) {
        case 1:
            register_code16(Z); // z
            break;

        case 2:
            tap_code16(LABK); // <
            break;

        case 3:
            tap_code16(RABK); // >
            break;

        default:
            break;
    }
}

void td_z_reset(tap_dance_state_t *state, void *build_info) {
    switch (state->count) {
        case 1:
            unregister_code16(Z);
    }
}

void td_spc_each(tap_dance_state_t *state, void *build_info) {
    unregister_code16(SPC);
    register_code16(SPC);

    if ((state->count % 2) == 0) {
        unregister_code16(SPC);
        tap_code16(SPC);
        register_code16(SPC);
    }
}

void td_spc_reset(tap_dance_state_t *state, void *build_info) {
    unregister_code16(SPC);
}

void td_grv_finished(tap_dance_state_t *state, void *build_info) {
    if (state->count == 1) {
        tap_code16(GRV);
        return;
    }

    // TODO: Change tap-hold decision so this does not work funny
    send_string_with_delay("``` " SS_LSFT("\n\n") "``` ", 10);
    tap_code16(UP);
}

tap_dance_action_t tap_dance_actions[] = {
    [_TD_NTIL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_ntil_finished, NULL),
    [_TD_Z]    = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_z_finished, td_z_reset),
    [_TD_SPC]  = ACTION_TAP_DANCE_FN_ADVANCED(td_spc_each, NULL, td_spc_reset),
    [_TD_GRV]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_grv_finished, NULL),
};
#endif

#if defined(KEY_OVERRIDE_ENABLE)
const key_override_t delete_key_override = ko_make_basic(MOD_MASK_SHIFT, BSPC, DEL);
const key_override_t volume_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_VOLU, KC_VOLD);
const key_override_t alt_f4_key_override = ko_make_basic(MOD_MASK_ALT, N4, LALT(F4));

const key_override_t *key_overrides[] = (const key_override_t *[]){
    &delete_key_override,
    &volume_key_override,
    &alt_f4_key_override,
};
#endif
