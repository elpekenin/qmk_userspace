// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Add IoT capabilities to our keyboard over UART :)
 */

// -- barrier --

#pragma once

#include <quantum/quantum.h>

/**
 * Framing protocol for UART messages.
 *
 * Each message is wrapped in a frame:
 *   [START=0xAA] [LEN] [...payload] [XOR] [END=0x55]
 */

#define M5_FRAME_START 0xAA
#define M5_FRAME_END 0x55

void m5_init(void);

void m5_send(const void *data, size_t data_len);
