// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

void sendchar_uart_init(void);

int8_t sendchar_uart(uint8_t chr);
