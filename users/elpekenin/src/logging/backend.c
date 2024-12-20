// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/logging/print.h>

#include "elpekenin/utils/sections.h"

#if defined(SEGGER_RTT_ENABLE)
#    include <lib/RTT/SEGGER_RTT.h>

static int8_t sendchar_rtt_hook(uint8_t c) {
#    if ENABLE_RTT == 1
    return SEGGER_RTT_PutChar(0, c);
#    else
    return 0;
#    endif
}
PEKE_SENDCHAR(sendchar_rtt_hook);
#endif

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
