// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Generic(ish) driver for touch screens.
 *
 * .. danger::
 *   Only ``XPT2046`` has been implemented/tested.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <stdbool.h>
#include <stdint.h>

// for pin_t
#include <platforms/gpio.h>
#include <platforms/chibios/gpio.h>

#include "elpekenin/utils/compiler.h"

/**
 * Current status of a screen.
 */
typedef struct PACKED {
    /**
     * Horizontal coordinate of the touch point.
     */
    uint16_t x;

    /**
     * Vertical coordinate of the touch point.
     */
    uint16_t y;

    /**
     * Whether it is pressed.
     */
    bool pressed;
} touch_report_t;

typedef enum { TOUCH_ROTATION_0, TOUCH_ROTATION_90, TOUCH_ROTATION_180, TOUCH_ROTATION_270 } touch_rotation_t;

/**
 * Configuration for SPI settings.
 */
typedef struct PACKED {
    /**
     * Its CS pin.
     */
    pin_t chip_select_pin;

    /**
     * Speed of the communications.
     */
    uint16_t divisor;

    /**
     * Byte order.
     */
    bool lsb_first;

    /**
     * SPI mode.
     */
    uint8_t mode;

    /**
     * Interrupt pin: flags that device is pressed.
     */
    pin_t irq_pin;

    /**
     * Command issued to get the X coordinate.
     */
    uint8_t x_cmd;

    /**
     * Command issued to get the Y coordinate.
     */
    uint8_t y_cmd;
} spi_touch_comms_config_t;

/**
 * Configuration for a touch device.
 */
typedef struct PACKED {
    /**
     * Horizontal size.
     */
    uint16_t width;

    /**
     * Vertical size.
     */
    uint16_t height;

    /**
     * Up-/down- scaling applied to raw X reading.
     */
    float scale_x;

    /**
     * Up-/down- scaling applied to raw Y reading.
     */
    float scale_y;

    /**
     * Offset applied to X reading (after being scaled).
     */
    int16_t offset_x;

    /**
     * Offset applied to Y reading (after being scaled).
     */
    int16_t offset_y;

    /**
     * Orientation of the device.
     */
    touch_rotation_t rotation;

    /**
     * Whether the device is flipped. Causes some maths changes.
     */
    bool upside_down;

    /**
     * Communications configuration.
     */
    spi_touch_comms_config_t spi_config;
} touch_driver_t;

/**
 * Handle to a device's configuration.
 */
typedef const void *touch_device_t;

/**
 * Initialize a device.
 */
bool touch_spi_init(touch_device_t device);

/**
 * (WEAK) Low-level function that performs math.
 *
 * Args:
 *     x: Raw X reading from sensor.
 *     y: Raw Y reading from sensor.
 *     driver: Handle to the device, to fetch its configuration.
 *     report: Output struct to be filled/updated.
 */
void report_from(int16_t x, int16_t y, touch_driver_t *driver, touch_report_t *report);

/**
 * Get the current state of a sensor.
 *
 * Args:
 *     device: Sensor's configuration.
 *     check_irq: Whether to check the IRQ's pin state.
 */
touch_report_t get_spi_touch_report(touch_device_t device, bool check_irq);
