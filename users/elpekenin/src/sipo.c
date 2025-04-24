// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/sipo.h"

#include "elpekenin/spi_custom.h"

#ifdef SIPO_DEBUG
#    include "quantum/logging/debug.h"
#    define sipo_dprintf dprintf
#else
#    define sipo_dprintf(...)
#endif

#define SIPO_BYTES ((N_SIPO_PINS + 7) / 8)

static uint8_t sipo_pin_state[SIPO_BYTES] = {0};
static bool    sipo_state_changed         = true;

static void print_sipo_status(void) {
    sipo_dprintf("MCU | ");

    for (uint8_t i = 0; i < SIPO_BYTES; ++i) {
// some GCC versions error due to %b but others don't (?)
// it is implemented in `printf`, so there's no problem
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

        sipo_dprintf("%b ", sipo_pin_state[SIPO_BYTES - i - 1]);

#pragma GCC diagnostic pop
    }

    sipo_dprintf("| END\n");
}

void set_sipo_pin(uint8_t position, bool state) {
    // this change makes position 0 to be the closest to the MCU, instead of being the 1st bit of the last byte
    uint8_t byte_offset = SIPO_BYTES - 1 - (position / 8);
    uint8_t bit_offset  = position % 8;

    // Check if pin already had that state
    uint8_t curr_value = (sipo_pin_state[byte_offset] >> bit_offset) & 1;
    if (curr_value == state) {
        sipo_dprintf("[INFO] %s: no changes\n", __func__);
        return;
    }

    sipo_state_changed = true;

    if (state)
        // add data starting on the least significant bit
        sipo_pin_state[byte_offset] |= (1 << bit_offset);
    else
        sipo_pin_state[byte_offset] &= ~(1 << bit_offset);
}

void send_sipo_state(void) {
    if (!sipo_state_changed) {
        sipo_dprintf("[INFO] %s: no changes\n", __func__);
        return;
    }

    sipo_state_changed = false;

    spi_custom_init(REGISTERS_SPI_DRIVER_ID);

    if (!spi_custom_start(SIPO_CS_PIN, false, REGISTERS_SPI_MODE, REGISTERS_SPI_DIV, REGISTERS_SPI_DRIVER_ID)) {
        sipo_dprintf("[ERROR] %s: (start SPI)\n", __func__);
        return;
    }

    gpio_write_pin_low(SIPO_CS_PIN);
    spi_custom_transmit(sipo_pin_state, SIPO_BYTES, REGISTERS_SPI_DRIVER_ID);
    gpio_write_pin_high(SIPO_CS_PIN);

    spi_custom_stop(REGISTERS_SPI_DRIVER_ID);

    print_sipo_status();
}
