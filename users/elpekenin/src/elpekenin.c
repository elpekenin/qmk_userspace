// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/allocator.h"
#include "elpekenin/crash.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/sections.h"
#include "elpekenin/signatures.h"
#include "elpekenin/string.h"
#include "generated/features.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/qp/assets.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

/**
 * Iterate the ``pre_init`` linker section, executing all functions put into it (initializers).
 *
 * Then, call :c:func:`keyboard_pre_init_keymap` for ``keymap.c``-level extensions.
 */
void keyboard_pre_init_user(void) {
    // PEKE_PRE_INIT
    FOREACH_SECTION(init_fn, pre_init, func) {
        (*func)();
    }

    keyboard_pre_init_keymap();
}

/**
 * First, check if previous execution crashed, printing traceback.
 *
 * Then, call :c:func:`keyboard_post_init_keymap` for ``keymap.c``-level extensions.
 *
 * Finally, iterate the ``post_init`` linker section, executing all functions put into it (initializers).
 */
void keyboard_post_init_user(void) {
    uint8_t      depth;
    const char  *msg;
    backtrace_t *call_stack = get_crash_call_stack(&depth, &msg);

    if (depth != 0) {
        logging(UNKNOWN, LOG_WARN, "Crash (%s)", msg);
        for (uint8_t i = 0; i < depth; ++i) {
            logging(UNKNOWN, LOG_ERROR, "%s", call_stack[i].name);
            logging(UNKNOWN, LOG_ERROR, "%p", call_stack[i].address);
        }
    }

    keyboard_post_init_keymap();

#if defined(AUTOCORRECT_ENABLE)
    autocorrect_enable();
#endif

#if defined(TRI_LAYER_ENABLE)
    set_tri_layer_lower_layer(_FN1);
    set_tri_layer_upper_layer(_FN2);
    set_tri_layer_adjust_layer(_RST);
#endif

    // PEKE_POST_INIT
    FOREACH_SECTION(init_fn, post_init, func) {
        (*func)();
    }
}

/**
 * Call :c:func:`shutdown_keymap` for ``keymap.c``-level customization
 *   If it returns ``false``, this function will exit.
 *   Otherwise, the default cleanup logic will be run.
 */
bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

    // power off all screens
#if defined(QUANTUM_PAINTER_ENABLE)
    for (uint8_t i = 0; i < qp_get_num_displays(); ++i) {
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
