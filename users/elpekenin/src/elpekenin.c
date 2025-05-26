// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/color.h>

#include "elpekenin/build_info.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backend.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/signatures.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/xap.h"

// compat: otherwise `Option()` is not defined
#if !defined(COMMUNITY_MODULE_CRASH_ENABLE)
#    error 'elpekenin/crash' must be enabled
#endif

#include "elpekenin/crash.h"

// compat: QFF/QGF files have `#include <qp.h>`, which fails if feature is disabled
#if defined(QUANTUM_PAINTER_ENABLE)
#    include "generated/qp_resources.h"
#endif

#if defined(RGB_MATRIX_ENABLE)
#    include <quantum/rgb_matrix/rgb_matrix.h>
#endif

// clang-format off
KEYCODE_STRING_NAMES_USER(
    KEYCODE_STRING_NAME(PK_CPYR),
    KEYCODE_STRING_NAME(PK_QCLR),
    KEYCODE_STRING_NAME(PK_KLOG),
    KEYCODE_STRING_NAME(PK_SIZE),
);
// clang-format on

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    // these have to happen as soon as possible, so that code relying on them doesn't break
    // then, keymap-level setup
    sendchar_init();

    build_info_init();

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
    if (IS_ENABLED(AUTOCORRECT)) {
        autocorrect_enable();
    }

    if (IS_ENABLED(COMMUNITY_MODULE_CRASH)) {
        Option(crash_info_t) maybe_crash = get_crash();

        if (maybe_crash.is_some) {
            crash_info_t crash = unwrap(maybe_crash);

            logging(LOG_WARN, "%s", crash.msg);
            for (uint8_t i = 0; i < crash.stack_depth; ++i) {
                logging(LOG_ERROR, "%s (%p)", crash.call_stack[i].name, crash.call_stack[i].address);
            }
        } else {
            logging(LOG_DEBUG, "Previous run did not crash");
        }
    }

    // compat: not visible if feature is off
#if defined(QUANTUM_PAINTER_ENABLE)
    load_qp_resources();
#endif

    if (IS_DEFINED(SPLIT_KEYBOARD)) {
        transactions_init();
    }

    if (IS_ENABLED(TRI_LAYER)) {
        set_tri_layer_lower_layer(FN1);
        set_tri_layer_upper_layer(FN2);
        set_tri_layer_adjust_layer(RST);
    }

    keyboard_post_init_keymap();
}

bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

    // power off all screens
    if (IS_ENABLED(QUANTUM_PAINTER)) {
        for (uint8_t i = 0; i < qp_get_num_devices(); ++i) {
            painter_device_t device = qp_get_device_by_index(i);
            qp_power(device, false);
        }
    }

    // compat: functions not visible if feature is off
#if defined(RGB_MATRIX_ENABLE)
    if (jump_to_bootloader) {
        // off for bootloader
        rgb_matrix_set_color_all(RGB_OFF);
    } else {
        // red for reboot
        rgb_matrix_set_color_all(RGB_MATRIX_MAXIMUM_BRIGHTNESS, 0, 0);
    }

    // flush
    rgb_matrix_update_pwm_buffers();
#endif

    // let host know
    if (IS_ENABLED(XAP)) {
        xap_shutdown(jump_to_bootloader);
    }

    return true;
}
