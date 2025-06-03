// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/sipo.h"

#include <platforms/wait.h>

#include "elpekenin/spi_custom.h"
#include "elpekenin/touch.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");

#include "elpekenin/logging.h"

bool touch_spi_init(touch_device_t device) {
    touch_driver_t          *driver       = (touch_driver_t *)device;
    spi_touch_comms_config_t comms_config = driver->spi_config;

    // Initialize the SPI peripheral
    spi_custom_init(TOUCH_SPI_DRIVER_ID);

    // Set up CS as output high
    set_sipo_pin(comms_config.chip_select_pin, true);
    send_sipo_state();

    // Set up IRQ as input
    if (comms_config.irq_pin != NO_PIN) {
        gpio_set_pin_input(comms_config.irq_pin);
    }

    return true;
}

bool touch_spi_start(spi_touch_comms_config_t comms_config) {
    return spi_custom_start(DUMMY_PIN, comms_config.lsb_first, comms_config.mode, comms_config.divisor, TOUCH_SPI_DRIVER_ID);
}

void touch_spi_stop(spi_touch_comms_config_t comms_config) {
    spi_custom_stop(TOUCH_SPI_DRIVER_ID);
    set_sipo_pin(comms_config.chip_select_pin, true);
    send_sipo_state();
}

static int16_t read_coord(uint8_t cmd, spi_touch_comms_config_t comms_config) {
    set_sipo_pin(comms_config.chip_select_pin, false);
    send_sipo_state();

    spi_custom_write(cmd, TOUCH_SPI_DRIVER_ID);
    int16_t coord = ((spi_custom_write(0, TOUCH_SPI_DRIVER_ID) << 8) | spi_custom_write(0, TOUCH_SPI_DRIVER_ID)) >> 3;

    set_sipo_pin(comms_config.chip_select_pin, true);

    return coord;
}

touch_report_t get_spi_touch_report(touch_device_t device, bool check_irq) {
    touch_driver_t          *driver       = (touch_driver_t *)device;
    spi_touch_comms_config_t comms_config = driver->spi_config;

    // Static variable so previous report is stored
    // Goal: When the screen is not pressed anymore, we can see the latest point pressed
    static touch_report_t report = {
        .x       = 0,
        .y       = 0,
        .pressed = false,
    };

    if (check_irq && comms_config.irq_pin != NO_PIN && readPin(comms_config.irq_pin)) {
        goto err;
    }

    if (!touch_spi_start(comms_config)) {
        logging(LOG_DEBUG, "Start comms");
        goto err;
    }

    report.pressed = true;

    // wait a bit so measurement is less noisy
    wait_ms(20);

    // Read data from sensor, 0-rotation based
    int16_t x = read_coord(driver->spi_config.x_cmd, comms_config);
    int16_t y = read_coord(driver->spi_config.y_cmd, comms_config);

    // Handles edge cases, scaling, offset, upside down & rotation
    report_from(x, y, driver, &report);

    touch_spi_stop(comms_config);

    return report;

err:
    report.pressed = false;
    return report;
}
