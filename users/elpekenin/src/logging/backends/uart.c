// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <drivers/uart.h>
#include <quantum/quantum.h>

#define BAUD_RATE 9600

void sendchar_uart_init(void) {
    uart_init(BAUD_RATE);

    // clear PuTTY serial monitor
    const uint8_t clear[] = {27, '[', '2', 'J'};
    uart_transmit(clear, ARRAY_SIZE(clear));

    const uint8_t cursor_home[] = {27, '[', 'H'};
    uart_transmit(cursor_home, ARRAY_SIZE(cursor_home));
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
