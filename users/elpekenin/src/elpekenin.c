// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/crash.h"
#include "elpekenin/logging.h"
#include "elpekenin/signatures.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/string.h"

#include "generated/features.h"

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"

PEKE_DEINIT(xap_shutdown, DEINIT_XAP);
#endif

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

/**
 * This file uses a bit of magic to invoke initializer and finalizer functions on QMK.
 *
 * They are stored in special linker sections. See :doc:`/userspace/utils/sections` for details.
 */

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
        for (int8_t i = 0; i < depth; ++i) {
            logging(UNKNOWN, LOG_ERROR, "%s", call_stack[i].name);
            logging(UNKNOWN, LOG_ERROR, "%p", call_stack[i].address);
        }
    }

    keyboard_post_init_keymap();

    // PEKE_POST_INIT
    FOREACH_SECTION(init_fn, post_init, func) {
        (*func)();
    }
}

/**
 * Call :c:func:`shutdown_keymap` for ``keymap.c``-level customization
 *   If it returns ``false``, this function will exit.
 *
 * Next, iterate the ``deinit`` linker section, executing all functions put into it (finalizers).
 */
bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

    // functions registered with PEKE_DEINIT
    FOREACH_SECTION(deinit_fn, deinit, func) {
        (*func)(jump_to_bootloader);
    }

    return true;
}
