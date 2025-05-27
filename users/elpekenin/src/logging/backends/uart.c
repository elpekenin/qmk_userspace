// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <drivers/uart.h>
#include <quantum/quantum.h>

typedef enum {
    CLIENT_NONE,
    CLIENT_PUTTY,
} uart_client_t;

typedef struct {
    const char *clear;
    const char *cursor;
} escape_sequences_t;

static const escape_sequences_t sequences[] = {
    [CLIENT_NONE] =
        {
            .clear  = NULL,
            .cursor = NULL,
        },
    [CLIENT_PUTTY] =
        {
            .clear  = "\x1B[2J",
            .cursor = "\x1B[H",
        },
};

static const escape_sequences_t sequence = sequences[UART_CLIENT];

void sendchar_uart_init(void) {
    uart_init(UART_LOG_BAUD_RATE);

    uart_transmit((const uint8_t *)sequence.clear, strlen(sequence.clear));
    uart_transmit((const uint8_t *)sequence.cursor, strlen(sequence.cursor));
}

int8_t sendchar_uart(uint8_t chr) {
    // TODO: buffering?

    // make PuTTY correctly print next line
    // without this, it doesn't roll back to start of line
    if (chr == '\n') {
        uart_write('\r');
    }

    uart_write(chr);
    return 0;
}
