// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Add IoT capabilities to our keyboard over UART :)
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>

void m5_init(void);

void m5_send(const void *data, size_t data_len);
