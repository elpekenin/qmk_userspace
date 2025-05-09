// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/logging/backends/uart.h"
#include "elpekenin/logging/backends/xap.h"

typedef void (*init_func_t)(void);

static init_func_t init_functions[] = {
#if defined(QUANTUM_PAINTER_ENABLE)
    sendchar_qp_init,
#endif

#if defined(UART_ENABLE)
    sendchar_uart_init,
#endif
};

static sendchar_func_t send_functions[] = {
    // default logging provided by QMK
    //    - USB via console endpoint, if CONSOLE_ENABLE
    //    - no-op otherwise
    sendchar,

#if defined(QUANTUM_PAINTER_ENABLE)
    sendchar_qp,
#endif

#if defined(SPLIT_KEYBOARD)
    sendchar_split,
#endif

#if defined(UART_ENABLE)
    sendchar_uart,
#endif

#if defined(XAP_ENABLE)
    sendchar_xap,
#endif
};

static int8_t user_sendchar(uint8_t chr) {
    for (size_t i = 0; i < ARRAY_SIZE(send_functions); ++i) {
        sendchar_func_t function = send_functions[i];

        // TODO: make something with returned value?
        // my custom hooks return 0 (so far)
        function(chr);
    }

    return 0;
}

void sendchar_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(init_functions); ++i) {
        init_func_t function = init_functions[i];

        function();
    }

    print_set_sendchar(user_sendchar);
}
