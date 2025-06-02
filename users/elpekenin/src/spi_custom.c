// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/* Copyright 2020 Nick Brassel (tzarc)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// TODO: Support for NO_PIN

#include "elpekenin/spi_custom.h"

#include <quantum/compiler_support.h>
#include <quantum/util.h>
#include <sys/cdefs.h>

#ifdef SPI_CUSTOM_DEBUG
#    include "quantum/logging/debug.h"
#    define spi_custom_dprintf dprintf
#else
#    define spi_custom_dprintf(...)
#endif

static SPIDriver *drivers[] = SPI_DRIVERS;
#define SPI_COUNT ARRAY_SIZE(drivers)
STATIC_ASSERT(SPI_COUNT <= UINT8_MAX, "too many buses defined");

static pin_t spi_sck_pins[SPI_COUNT]   = SPI_SCK_PINS;
static pin_t spi_mosi_pins[SPI_COUNT]  = SPI_MOSI_PINS;
static pin_t spi_miso_pins[SPI_COUNT]  = SPI_MISO_PINS;
static pin_t spi_slave_pins[SPI_COUNT] = {[0 ... SPI_COUNT - 1] = NO_PIN};

static SPIConfig spi_configs[SPI_COUNT];

static bool is_initialised[SPI_COUNT] = {[0 ... SPI_COUNT - 1] = false};

// elements in this array are created during `spi_custom_init`
static mutex_t spi_mutexes[SPI_COUNT];

__weak_symbol void spi_custom_init(uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return;
    }

    if (!is_initialised[n]) {
        is_initialised[n] = true;
        spi_mutexes[n]    = (mutex_t)__MUTEX_DATA(spi_mutexes[n]);

#if defined(K20x) || defined(KL2x) || defined(RP2040)
        spi_configs[n] = (SPIConfig){NULL, 0, 0, 0};
#else
        spi_configs[n] = (SPIConfig){false, NULL, 0, 0, 0, 0};
#endif

        // Try releasing special pins for a short time
        gpio_set_pin_input(spi_sck_pins[n]);
        gpio_set_pin_input(spi_mosi_pins[n]);
        gpio_set_pin_input(spi_miso_pins[n]);

        chThdSleepMilliseconds(10);
#if defined(USE_GPIOV1)
        palSetPadMode(PAL_PORT(spi_sck_pins[n]), PAL_PAD(spi_sck_pins[n]), SPI_SCK_PAL_MODE);
        palSetPadMode(PAL_PORT(spi_mosi_pins[n]), PAL_PAD(spi_mosi_pins[n]), SPI_MOSI_PAL_MODE);
        palSetPadMode(PAL_PORT(spi_miso_pins[n]), PAL_PAD(spi_miso_pins[n]), SPI_MISO_PAL_MODE);
#else
        palSetPadMode(PAL_PORT(spi_sck_pins[n]), PAL_PAD(spi_sck_pins[n]), SPI_SCK_FLAGS);
        palSetPadMode(PAL_PORT(spi_mosi_pins[n]), PAL_PAD(spi_mosi_pins[n]), SPI_MOSI_FLAGS);
        palSetPadMode(PAL_PORT(spi_miso_pins[n]), PAL_PAD(spi_miso_pins[n]), SPI_MISO_FLAGS);
#endif
        spiStop(drivers[n]);
        spi_slave_pins[n] = NO_PIN;
    }
}

bool spi_custom_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor, uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return false;
    }

    if (spi_slave_pins[n] != NO_PIN || slavePin == NO_PIN) {
        spi_custom_dprintf("[ERROR] %s: invalid CS settings\n", __func__);
        return false;
    }

    if (!chMtxTryLock(&spi_mutexes[n])) {
        spi_custom_dprintf("[ERROR] %s: could not lock\n", __func__);
        return false;
    }

#if !(defined(WB32F3G71xx) || defined(WB32FQ95xx))
    uint16_t roundedDivisor = 2;
    while (roundedDivisor < divisor) {
        roundedDivisor <<= 1;
    }

    if (roundedDivisor < 2 || roundedDivisor > 256) {
        spi_custom_dprintf("[ERROR] %s: invalid divisor %d\n", __func__, divisor);
        goto err;
    }
#endif

#if defined(K20x) || defined(KL2x)
    spi_configs[n].tar0 = SPIx_CTARn_FMSZ(7) | SPIx_CTARn_ASC(1);

    if (lsbFirst) {
        spi_configs[n].tar0 |= SPIx_CTARn_LSBFE;
    }

    switch (mode) {
        case 0:
            break;
        case 1:
            spi_configs[n].tar0 |= SPIx_CTARn_CPHA;
            break;
        case 2:
            spi_configs[n].tar0 |= SPIx_CTARn_CPOL;
            break;
        case 3:
            spi_configs[n].tar0 |= SPIx_CTARn_CPHA | SPIx_CTARn_CPOL;
            break;
    }

    switch (roundedDivisor) {
        case 2:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(0);
            break;
        case 4:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(1);
            break;
        case 8:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(3);
            break;
        case 16:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(4);
            break;
        case 32:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(5);
            break;
        case 64:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(6);
            break;
        case 128:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(7);
            break;
        case 256:
            spi_configs[n].tar0 |= SPIx_CTARn_BR(8);
            break;
    }

#elif defined(HT32)
    spi_configs[n].cr0 = SPI_CR0_SELOEN;
    spi_configs[n].cr1 = SPI_CR1_MODE | 8; // 8 bits and in master mode

    if (lsbFirst) {
        spi_configs[n].cr1 |= SPI_CR1_FIRSTBIT;
    }

    switch (mode) {
        case 0:
            spi_configs[n].cr1 |= SPI_CR1_FORMAT_MODE0;
            break;
        case 1:
            spi_configs[n].cr1 |= SPI_CR1_FORMAT_MODE1;
            break;
        case 2:
            spi_configs[n].cr1 |= SPI_CR1_FORMAT_MODE2;
            break;
        case 3:
            spi_configs[n].cr1 |= SPI_CR1_FORMAT_MODE3;
            break;
    }

    spi_configs[n].cpr = (roundedDivisor - 1) >> 1;

#elif defined(WB32F3G71xx) || defined(WB32FQ95xx)
    if (!lsbFirst) {
        osalDbgAssert(lsbFirst != FALSE, "unsupported lsbFirst");
    }

    if (divisor < 1) {
        goto err;
    }

    spi_configs[n].SPI_BaudRatePrescaler = (divisor << 2);

    switch (mode) {
        case 0:
            spi_configs[n].SPI_CPHA = SPI_CPHA_1Edge;
            spi_configs[n].SPI_CPOL = SPI_CPOL_Low;
            break;
        case 1:
            spi_configs[n].SPI_CPHA = SPI_CPHA_2Edge;
            spi_configs[n].SPI_CPOL = SPI_CPOL_Low;
            break;
        case 2:
            spi_configs[n].SPI_CPHA = SPI_CPHA_1Edge;
            spi_configs[n].SPI_CPOL = SPI_CPOL_High;
            break;
        case 3:
            spi_configs[n].SPI_CPHA = SPI_CPHA_2Edge;
            spi_configs[n].SPI_CPOL = SPI_CPOL_High;
            break;
    }
#elif defined(MCU_RP)
    if (lsbFirst) {
        osalDbgAssert(lsbFirst == false, "RP2040s PrimeCell SPI implementation does not support sending LSB first.");
        spi_custom_dprintf("[ERROR] %s: invalid LSB\n", __func__);
    }

    // Motorola frame format and 8bit transfer data size.
    spi_configs[n].SSPCR0 = SPI_SSPCR0_FRF_MOTOROLA | SPI_SSPCR0_DSS_8BIT;
    // Serial output clock = (ck_sys or ck_peri) / (SSPCPSR->CPSDVSR * (1 +
    // SSPCR0->SCR)). SCR is always set to zero, as QMK SPI API expects the
    // passed divisor to be the only value to divide the input clock by.
    spi_configs[n].SSPCPSR = roundedDivisor; // Even number from 2 to 254

    switch (mode) {
        case 0:
            spi_configs[n].SSPCR0 &= ~SPI_SSPCR0_SPO; // Clock polarity: low
            spi_configs[n].SSPCR0 &= ~SPI_SSPCR0_SPH; // Clock phase: sample on first edge
            break;
        case 1:
            spi_configs[n].SSPCR0 &= ~SPI_SSPCR0_SPO; // Clock polarity: low
            spi_configs[n].SSPCR0 |= SPI_SSPCR0_SPH;  // Clock phase: sample on second edge transition
            break;
        case 2:
            spi_configs[n].SSPCR0 |= SPI_SSPCR0_SPO;  // Clock polarity: high
            spi_configs[n].SSPCR0 &= ~SPI_SSPCR0_SPH; // Clock phase: sample on first edge
            break;
        case 3:
            spi_configs[n].SSPCR0 |= SPI_SSPCR0_SPO; // Clock polarity: high
            spi_configs[n].SSPCR0 |= SPI_SSPCR0_SPH; // Clock phase: sample on second edge transition
            break;
    }
#else
    spi_configs[n].cr1 = 0;

    if (lsbFirst) {
        spi_configs[n].cr1 |= SPI_CR1_LSBFIRST;
    }

    switch (mode) {
        case 0:
            break;
        case 1:
            spi_configs[n].cr1 |= SPI_CR1_CPHA;
            break;
        case 2:
            spi_configs[n].cr1 |= SPI_CR1_CPOL;
            break;
        case 3:
            spi_configs[n].cr1 |= SPI_CR1_CPHA | SPI_CR1_CPOL;
            break;
    }

    switch (roundedDivisor) {
        case 2:
            break;
        case 4:
            spi_configs[n].cr1 |= SPI_CR1_BR_0;
            break;
        case 8:
            spi_configs[n].cr1 |= SPI_CR1_BR_1;
            break;
        case 16:
            spi_configs[n].cr1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0;
            break;
        case 32:
            spi_configs[n].cr1 |= SPI_CR1_BR_2;
            break;
        case 64:
            spi_configs[n].cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0;
            break;
        case 128:
            spi_configs[n].cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;
            break;
        case 256:
            spi_configs[n].cr1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;
            break;
    }
#endif

    spi_slave_pins[n]     = slavePin;
    spi_configs[n].ssport = PAL_PORT(slavePin);
    spi_configs[n].sspad  = PAL_PAD(slavePin);

    gpio_set_pin_output(slavePin);
    spiStart(drivers[n], &spi_configs[n]);
    spiSelect(drivers[n]);

    return true;

err:
    chMtxUnlock(&spi_mutexes[n]);
    return false;
}

spi_status_t spi_custom_write(uint8_t data, uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return SPI_STATUS_ERROR;
    }

    uint8_t rxData;
    spiExchange(drivers[n], 1, &data, &rxData);

    return rxData;
}

spi_status_t spi_custom_read(uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return SPI_STATUS_ERROR;
    }

    uint8_t data = 0;
    spiReceive(drivers[n], 1, &data);

    return data;
}

spi_status_t spi_custom_transmit(const uint8_t *data, uint16_t length, uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return SPI_STATUS_ERROR;
    }

    spiSend(drivers[n], length, data);
    return SPI_STATUS_SUCCESS;
}

spi_status_t spi_custom_receive(uint8_t *data, uint16_t length, uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return SPI_STATUS_ERROR;
    }

    spiReceive(drivers[n], length, data);
    return SPI_STATUS_SUCCESS;
}

void spi_custom_stop(uint8_t n) {
    if (n >= SPI_COUNT) {
        spi_custom_dprintf("[ERROR] %s: n==%d invalid\n", __func__, n);
        return;
    }

    if (spi_slave_pins[n] != NO_PIN) {
        spiUnselect(drivers[n]);
        spiStop(drivers[n]);
        spi_slave_pins[n] = NO_PIN;
    }

    chMtxUnlock(&spi_mutexes[n]);
}
