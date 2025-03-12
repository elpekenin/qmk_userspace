// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/utils/sections.h"

// default logging provided by QMK
//    - USB via console endpoint, if CONSOLE_ENABLE
//    - no-op otherwise
PEKE_SENDCHAR(sendchar);

static int8_t user_sendchar(uint8_t c) {
    FOREACH_SECTION(sendchar_func_t, sendchar, func) {
        (*func)(c);
    }

    return 0;
}

static void init_sendchar(void) {
#if ENABLE_SENDCHAR == 1
    print_set_sendchar(user_sendchar);
#else
    (void)user_sendchar;
#endif
}
PEKE_PRE_INIT(init_sendchar, INIT_SENDCHAR);
