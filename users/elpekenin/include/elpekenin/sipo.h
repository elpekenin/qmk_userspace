// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// create pins list
#define configure_sipo_pins(pin_names...)           \
    enum { pin_names, __SIPO_PINS }; \
    _Static_assert(__SIPO_PINS <= N_SIPO_PINS, "Defined more pin names than the amount configured")

// control the buffer
void set_sipo_pin(uint8_t position, bool state);

// send status
void send_sipo_state(void);
