// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin.h"
#include "elpekenin/crash.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/string.h"

#include "generated/features.h"

extern void elpekenin_zig_init(void);
PEKE_PRE_INIT(elpekenin_zig_init, INIT_ZIG);

extern void elpekenin_zig_deinit(bool);
PEKE_DEINIT(elpekenin_zig_deinit, DEINIT_ZIG);

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

/**
 * Iterate the :c:var:`pre_init` linker section, executing all functions put into it (initializers).
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
 * Then, call :c:func:`keyboard_post_init_user` for ``keymap.c``-level extensions.
 *
 * Finally, iterate the :c:var:`post_init` linker section, executing all functions put into it (initializers).
 */
void keyboard_post_init_user(void) {
    if (program_crashed()) {
        print_crash_call_stack();
        clear_crash_info();
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
 * Next, iterate the :c:var:`deinit` linker section, executing all functions put into it (finalizers).
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
