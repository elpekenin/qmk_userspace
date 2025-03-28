// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/logging/backends/xap.h"

static sendchar_func_t hooks[] = {
    // default logging provided by QMK
    //    - USB via console endpoint, if CONSOLE_ENABLE
    //    - no-op otherwise
    sendchar,

#if defined(QUANTUM_PAINTER_ENABLE)
    sendchar_qp_hook,
#endif

#if defined(SPLIT_KEYBOARD)
    sendchar_split_hook,
#endif

#if defined(XAP_ENABLE)
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

void sendchar_init(void) {
    print_set_sendchar(user_sendchar);
}
