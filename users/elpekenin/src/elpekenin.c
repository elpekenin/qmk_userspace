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

void keyboard_pre_init_user(void) {
    // PEKE_PRE_INIT
    FOREACH_SECTION(init_fn, pre_init, func) {
        (*func)();
    }

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
    keyboard_post_init_keymap();

    if (program_crashed()) {
        print_crash_call_stack();
        clear_crash_info();
    }

    // PEKE_POST_INIT
    FOREACH_SECTION(init_fn, post_init, func) {
        (*func)();
    }
}

