// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/build_info.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/signatures.h"
#include "elpekenin/string.h"
#include "generated/features.h"

#if defined(COMMUNITY_MODULE_CRASH_ENABLE)
#    include "elpekenin/crash.h"
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/logging/backends/qp.h"
#    include "elpekenin/qp/assets.h"
#    include "elpekenin/qp/graphics.h"
#endif

#if defined(SPLIT_KEYBOARD)
#    include "elpekenin/split/transactions.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    // these have to happen as soon as possible, so that code relying on them doesn't break
    // then, keymap-level setup
    qp_log_init();

#if ENABLE_SENDCHAR == 1
    sendchar_init();
#endif

    build_info_init();

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
#if defined(COMMUNITY_MODULE_CRASH_ENABLE)
    uint8_t      depth;
    const char  *msg;
    backtrace_t *call_stack = get_crash_call_stack(&depth, &msg);

    if (depth != 0) {
        logging(UNKNOWN, LOG_WARN, "Crash (%s)", msg);
        for (uint8_t i = 0; i < depth; ++i) {
            logging(UNKNOWN, LOG_ERROR, "%s (%p)", call_stack[i].name, call_stack[i].address);
        }
    }
#endif

#if defined(AUTOCORRECT_ENABLE)
    autocorrect_enable();
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
    qp_tasks_init();
#endif

#if defined(SPLIT_KEYBOARD)
    transactions_init();
#endif

#if defined(TRI_LAYER_ENABLE)
    set_tri_layer_lower_layer(_FN1);
    set_tri_layer_upper_layer(_FN2);
    set_tri_layer_adjust_layer(_RST);
#endif

    keyboard_post_init_keymap();
}

bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

    // power off all screens
#if defined(QUANTUM_PAINTER_ENABLE)
    for (uint8_t i = 0; i < qp_get_num_devices(); ++i) {
        painter_device_t device = qp_get_device_by_index(i);
        qp_power(device, false);
    }
#endif

#if defined(RGB_MATRIX_ENABLE)
    if (jump_to_bootloader) {
        // off for bootloader
        rgb_matrix_set_color_all(RGB_OFF);
    } else {
        // red for reboot
        rgb_matrix_set_color_all(RGB_MATRIX_MAXIMUM_BRIGHTNESS, 0, 0);
    }

    // flush
    void rgb_matrix_update_pwm_buffers(void);
    rgb_matrix_update_pwm_buffers();
#endif

    // let host know
#if defined(XAP_ENABLE)
    xap_shutdown(jump_to_bootloader);
#endif

    return true;
}
