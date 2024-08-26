// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"
#include "elpekenin/sipo.h"
#include "elpekenin/spi_custom.h"
#include "elpekenin/utils/shortcuts.h"

#define SIPO_BYTES ((N_SIPO_PINS + 7) / 8)

static uint8_t sipo_pin_state[SIPO_BYTES] = {0};
static bool    sipo_state_changed          = true;

static inline void print_sipo_byte(uint8_t x) {
    uint8_t byte = sipo_pin_state[x];

    _ = logging(
        SIPO,
        LOG_DEBUG,
        "%d%d%d%d%d%d%d%d",
        GET_BIT(byte, 0),
        GET_BIT(byte, 1),
        GET_BIT(byte, 2),
        GET_BIT(byte, 3),
        GET_BIT(byte, 4),
        GET_BIT(byte, 5),
        GET_BIT(byte, 6),
        GET_BIT(byte, 7)
    );
}

static inline void print_sipo_status(void) {
    _ = logging(SIPO, LOG_DEBUG, "MCU");

    for (int i = (SIPO_BYTES - 1); i >= 0; --i) {
        print_sipo_byte(i);
    }

    _ = logging(SIPO, LOG_DEBUG, "END");
}

void set_sipo_pin(uint8_t  position, bool state) {
    // this change makes position 0 to be the closest to the MCU, instead of being the 1st bit of the last byte
    uint8_t byte_offset = SIPO_BYTES - 1 - (position / 8);
    uint8_t bit_offset  = position % 8;

    // Check if pin already had that state
    uint8_t curr_value = (sipo_pin_state[byte_offset] >> bit_offset) & 1;
    if (curr_value == state) {
        _ = logging(SIPO, LOG_DEBUG, "%s: no changes", __func__);
        return;
    }

    sipo_state_changed = true;

    if (state)
        // add data starting on the least significant bit
        sipo_pin_state[byte_offset] |=  (1 << bit_offset);
    else
        sipo_pin_state[byte_offset] &= ~(1 << bit_offset);
}

void send_sipo_state(void) {
    if (!sipo_state_changed) {
        _ = logging(SIPO, LOG_DEBUG, "%s: no changes", __func__);
        return;
    }

    sipo_state_changed = false;

    spi_custom_init(REGISTERS_SPI_DRIVER_ID);

    if(!spi_custom_start(SIPO_CS_PIN, false, REGISTERS_SPI_MODE, REGISTERS_SPI_DIV, REGISTERS_SPI_DRIVER_ID)) {
        _ = logging(SIPO, LOG_ERROR, "%s (init SPI)", __func__);
        return;
    }

    gpio_write_pin_low(SIPO_CS_PIN);
    spi_custom_transmit(sipo_pin_state, SIPO_BYTES, REGISTERS_SPI_DRIVER_ID);
    gpio_write_pin_high(SIPO_CS_PIN);

    spi_custom_stop(REGISTERS_SPI_DRIVER_ID);

    print_sipo_status();
}
