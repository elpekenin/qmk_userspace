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

    // Wrap payload in a framed message so the receiver can detect
    // message boundaries and resync after data loss.
    //
    // Frame: [START] [LEN] [payload...] [XOR of payload] [END]
    uint8_t frame[4 + data_len];

    frame[0] = M5_FRAME_START;
    frame[1] = (uint8_t)data_len;

    uint8_t xor_sum = 0;

    // copy data and compute XOR at the same time
    for (size_t i = 0; i < data_len; ++i) {
        frame[2 + i] = ptr[i];
        xor_sum ^= ptr[i];
    }

    frame[2 + data_len] = xor_sum;
    frame[3 + data_len] = M5_FRAME_END;

    uart_transmit(frame, 4 + data_len);
}
