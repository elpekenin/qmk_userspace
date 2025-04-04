// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Driver for 74HC595 Serial In - Parallel Out shift registers.
 */

// -- barrier --

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Configure a list of "pin" names, aka: identifiers for register(s) output.
 *
 * .. caution::
 *   You will get a build error if :c:macro:`N_SIPO_PINS` is not correctly set up.
 */
#define configure_sipo_pins(pin_names...) \
    enum { pin_names, __SIPO_PINS };      \
    _Static_assert(__SIPO_PINS <= N_SIPO_PINS, "Defined more pin names than the amount configured")

/**
 * Update the state of a pin in the internal buffer.
 *
 * .. hint::
 *   This will not apply the setting yet, because of performance issues.
 *
 *   You should call :c:func:`send_sipo_state` to get it flushed.
 *
 * Args:
 *     pin: The name defined on :c:macro:`configure_sipo_pins`.
 *     state: Whether low or high output is desired.
 */
void set_sipo_pin(uint8_t pin, bool state);

/**
 * Flush the internal buffer into the hardware, to actually get outputs
 * set low/high as desired.
 */
void send_sipo_state(void);
