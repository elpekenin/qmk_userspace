// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/logging/backends/rtt.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/logging/backends/xap.h"
#include "elpekenin/sections.h"

static sendchar_func_t hooks[] = {
    // default logging provided by QMK
    //    - USB via console endpoint, if CONSOLE_ENABLE
    //    - no-op otherwise
    sendchar,

#ifdef QUANTUM_PAINTER_ENABLE
    sendchar_qp_hook,
#endif

#ifdef RTT_DRIVER_ENABLE
    sendchar_rtt_hook,
#endif

#ifdef SPLIT_KEYBOARD
    sendchar_split_hook,
#endif

#ifdef XAP_ENABLE
    sendchar_xap_hook,
#endif
};

static int8_t user_sendchar(uint8_t c) {
    for (uint8_t i = 0; i < ARRAY_SIZE(hooks); ++i) {
        sendchar_func_t hook = hooks[i];

        // TODO: make something with returned value?
        // my custom hooks return 0 (so far)
        hook(c);
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
