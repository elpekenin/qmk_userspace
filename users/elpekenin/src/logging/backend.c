// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/logging/backends/uart.h"
#include "elpekenin/logging/backends/xap.h"

typedef void (*init_func_t)(void);

static init_func_t init_functions[] = {
#if IS_ENABLED(QP_LOG)
    sendchar_qp_init,
#endif

#if IS_ENABLED(UART_LOG)
    sendchar_uart_init,
#endif
};

static sendchar_func_t send_functions[] = {
    // default logging provided by QMK
    //    - USB via console endpoint, if CONSOLE_ENABLE
    //    - no-op otherwise
    sendchar,

#if IS_ENABLED(QP_LOG)
    sendchar_qp,
#endif

#if IS_ENABLED(SPLIT_LOG)
    sendchar_split,
#endif

#if IS_ENABLED(UART_LOG)
    sendchar_uart,
#endif

#if IS_ENABLED(XAP_LOG)
    sendchar_xap,
#endif
};

static int8_t user_sendchar(uint8_t chr) {
    for (size_t i = 0; i < ARRAY_SIZE(send_functions); ++i) {
        sendchar_func_t function = send_functions[i];

        const int8_t ret = function(chr);
        if (ret < 0) {
            // error
        }
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
