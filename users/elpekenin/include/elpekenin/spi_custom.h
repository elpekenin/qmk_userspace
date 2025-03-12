/* Copyright 2020 Nick Brassel (tzarc)
 * Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
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

/**
 * Some patching on QMK's ``spi_master.h`` driver, to allow using multiple buses.
 *
 * .. caution::
 *   The following must be set up:
 *    * :c:macro:`SPI_DRIVERS`
 *    * :c:macro:`SPI_SCK_PINS`
 *    * :c:macro:`SPI_MOSI_PINS`
 *    * :c:macro:`SPI_MISO_PINS`
 *
 *   They will be automatically configured with their single-driver counterparts, if available.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <ch.h>
#include <hal.h>
#include <platforms/chibios/chibios_config.h>
#include <platforms/chibios/gpio.h>
#include <platforms/gpio.h>

#include "elpekenin/utils/compiler.h"

#ifndef SPI_DRIVERS
#    if defined(SPI_DRIVER)
#        define SPI_DRIVERS {&SPI_DRIVER}
#    else
#        error "Select drivers for SPI"
#    endif
#endif

#define SPI_COUNT (ARRAY_SIZE((SPIDriver *[])SPI_DRIVERS))

#if !defined(SPI_SCK_PINS)
#    if defined(SPI_SCK_PIN)
#        define SPI_SCK_PINS {SPI_SCK_PIN}
#    else
#        error "Select pins for SCK"
#    endif
#endif

#if !defined(SPI_MOSI_PINS)
#    if defined(SPI_MOSI_PIN)
#        define SPI_MOSI_PINS {SPI_MOSI_PIN}
#    else
#        error "Select pins for MOSI"
#    endif
#endif

#ifndef SPI_MISO_PINS
#    if defined(SPI_MISO_PIN)
#        define SPI_MISO_PINS {SPI_MISO_PIN}
#    else
#        error "Select pins for MISO"
#    endif
#endif

#ifndef SPI_SCK_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_SCK_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_SCK_PAL_MODE 5
#    endif
#endif

#ifndef SPI_MOSI_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_MOSI_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_MOSI_PAL_MODE 5
#    endif
#endif

#ifndef SPI_MISO_PAL_MODE
#    if defined(USE_GPIOV1)
#        define SPI_MISO_PAL_MODE PAL_MODE_ALTERNATE_PUSHPULL
#    else
#        define SPI_MISO_PAL_MODE 5
#    endif
#endif

/** */
typedef enum {
    /** */
    SPI_STATUS_SUCCESS =
#define SPI_STATUS_SUCCESS (0)
        SPI_STATUS_SUCCESS,

    /** */
    SPI_STATUS_ERROR =
#define SPI_STATUS_ERROR (-1)
        SPI_STATUS_ERROR,

    /** */
    SPI_STATUS_TIMEOUT =
#define SPI_STATUS_TIMEOUT (-2)
        SPI_STATUS_TIMEOUT,
} spi_status_t;

#define SPI_TIMEOUT_IMMEDIATE (0)
#define SPI_TIMEOUT_INFINITE (0xFFFF)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the ``n``'th driver.
 */
void spi_custom_init(uint8_t n);

/**
 * Set up the ``n``'th driver for a transmission.
 *
 * Args:
 *     slavePin: The chip select pin for the target device.
 *     lsbFirst: Whether Least Significant Bit is sent first or last.
 *     mode: SPI clocks' mode (0-3).
 *     divisor: Control the clock's speed.
 *     n: Index of the driver to be used.
 *
 * Return:
 *     Whether operation was successful.
 */
bool spi_custom_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor, uint8_t n);

/**
 * Send a single byte (``data``) over the ``n``'th driver.
 */
spi_status_t spi_custom_write(uint8_t data, uint8_t n);

/**
 * Read a single byte (return) over the ``n``'th driver.
 */
spi_status_t spi_custom_read(uint8_t n);

/**
 * Send ``length`` bytes from ``data`` over the ``n``'th driver.
 */
spi_status_t spi_custom_transmit(const uint8_t *data, uint16_t length, uint8_t n);

/**
 * Read ``length`` bytes into ``data`` over the ``n``'th driver.
 */
spi_status_t spi_custom_receive(uint8_t *data, uint16_t length, uint8_t n);

/**
 * Undo the settings performced by :c:func:`spi_custom_start`
 */
void spi_custom_stop(uint8_t n);
#ifdef __cplusplus
}
#endif
