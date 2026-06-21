// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/m5.h"

#include "uart.h"

void m5_init(void) {
    uart_init(M5_BAUD_RATE);
}

void m5_send(const void *data, size_t data_len) {
    const uint8_t *ptr = data;
    if (IS_ENABLED(M5_DEBUG)) {
        printf("[m5] sending: {");
        for (size_t i = 0; i < data_len; ++i) {
            printf(" %d", ptr[i]);
        }
        printf(" }\n");
    }

    uart_transmit(ptr, data_len);
}
