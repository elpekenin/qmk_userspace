// Copyright 2021 Nick Brassel (@tzarc)
// Copyright 2023 Pablo Martinez (@elpekenin)
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef QUANTUM_PAINTER_SPI_ENABLE

#    include "qp_comms_spi.h"

#    include "elpekenin/sipo.h"
#    include "elpekenin/spi_custom.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base SPI support

bool qp_comms_spi_init(painter_device_t device) {
    painter_driver_t      *driver       = (painter_driver_t *)device;
    qp_comms_spi_config_t *comms_config = (qp_comms_spi_config_t *)driver->comms_config;

    // Initialize the SPI peripheral
    spi_custom_init(SCREENS_SPI_DRIVER_ID);

    // Set up CS as output high
    set_sipo_pin(comms_config->chip_select_pin, true);
    send_sipo_state();

    return true;
}

bool qp_comms_spi_start(painter_device_t device) {
    painter_driver_t      *driver       = (painter_driver_t *)device;
    qp_comms_spi_config_t *comms_config = (qp_comms_spi_config_t *)driver->comms_config;
    return spi_custom_start(DUMMY_PIN, comms_config->lsb_first, comms_config->mode, comms_config->divisor, SCREENS_SPI_DRIVER_ID);
}

uint32_t qp_comms_spi_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    uint32_t       bytes_remaining = byte_count;
    const uint8_t *p               = (const uint8_t *)data;
    uint32_t       max_msg_length  = 1024;

    while (bytes_remaining > 0) {
        uint32_t bytes_this_loop = MIN(bytes_remaining, max_msg_length);
        spi_custom_transmit(p, bytes_this_loop, SCREENS_SPI_DRIVER_ID);
        p += bytes_this_loop;
        bytes_remaining -= bytes_this_loop;
    }

    return byte_count - bytes_remaining;
}

void qp_comms_spi_stop(painter_device_t device) {
    painter_driver_t      *driver       = (painter_driver_t *)device;
    qp_comms_spi_config_t *comms_config = (qp_comms_spi_config_t *)driver->comms_config;

    spi_custom_stop(SCREENS_SPI_DRIVER_ID);

    set_sipo_pin(comms_config->chip_select_pin, true);
    send_sipo_state();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI with D/C and RST pins

#    ifdef QUANTUM_PAINTER_SPI_DC_RESET_ENABLE

bool qp_comms_spi_dc_reset_init(painter_device_t device) {
    if (!qp_comms_spi_init(device)) {
        return false;
    }

    painter_driver_t               *driver       = (painter_driver_t *)device;
    qp_comms_spi_dc_reset_config_t *comms_config = (qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    // Set up D/C as output low, if specified
    if (comms_config->dc_pin != NO_PIN) {
        set_sipo_pin(comms_config->dc_pin, false);
        send_sipo_state();
    }

    // Use RST as to perform a reset
    if (comms_config->reset_pin != NO_PIN) {
        set_sipo_pin(comms_config->reset_pin, false);
        send_sipo_state();
        wait_ms(20);

        set_sipo_pin(comms_config->reset_pin, true);
        send_sipo_state();
        wait_ms(20);
    }

    return true;
}

uint32_t qp_comms_spi_dc_reset_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    painter_driver_t               *driver       = (painter_driver_t *)device;
    qp_comms_spi_dc_reset_config_t *comms_config = (qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    set_sipo_pin(comms_config->dc_pin, true);
    set_sipo_pin(comms_config->spi_config.chip_select_pin, false);
    send_sipo_state();

    uint32_t ret = qp_comms_spi_send_data(device, data, byte_count);

    set_sipo_pin(comms_config->spi_config.chip_select_pin, true);
    send_sipo_state();

    return ret;
}

void qp_comms_spi_dc_reset_send_command(painter_device_t device, uint8_t cmd) {
    painter_driver_t               *driver       = (painter_driver_t *)device;
    qp_comms_spi_dc_reset_config_t *comms_config = (qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    set_sipo_pin(comms_config->dc_pin, false);
    set_sipo_pin(comms_config->spi_config.chip_select_pin, false);
    send_sipo_state();

    spi_custom_write(cmd, SCREENS_SPI_DRIVER_ID);

    set_sipo_pin(comms_config->spi_config.chip_select_pin, true);
    send_sipo_state();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI with D/C and RST pins but sending one byte at a time, needed for some devices

void qp_comms_spi_dc_reset_single_byte_send_command(painter_device_t device, uint8_t cmd) {
    painter_driver_t               *driver       = (painter_driver_t *)device;
    qp_comms_spi_dc_reset_config_t *comms_config = (qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    set_sipo_pin(comms_config->dc_pin, false);
    set_sipo_pin(comms_config->spi_config.chip_select_pin, false);
    send_sipo_state();

    spi_custom_write(cmd, SCREENS_SPI_DRIVER_ID);

    set_sipo_pin(comms_config->spi_config.chip_select_pin, true);
    send_sipo_state();
}

uint32_t qp_comms_spi_dc_reset_single_byte_send_data(painter_device_t device, const void *data, uint32_t byte_count) {
    painter_driver_t               *driver       = (painter_driver_t *)device;
    qp_comms_spi_dc_reset_config_t *comms_config = (qp_comms_spi_dc_reset_config_t *)driver->comms_config;

    uint32_t       bytes_remaining = byte_count;
    const uint8_t *p               = (const uint8_t *)data;
    uint32_t       max_msg_length  = 1;

    set_sipo_pin(comms_config->dc_pin, true);
    while (bytes_remaining > 0) {
        uint32_t bytes_this_loop = MIN(bytes_remaining, max_msg_length);

        set_sipo_pin(comms_config->spi_config.chip_select_pin, false);
        send_sipo_state();

        spi_custom_transmit(p, bytes_this_loop, SCREENS_SPI_DRIVER_ID);

        set_sipo_pin(comms_config->spi_config.chip_select_pin, true);
        send_sipo_state();

        p += bytes_this_loop;
        bytes_remaining -= bytes_this_loop;
    }

    return byte_count - bytes_remaining;
}

void qp_comms_spi_dc_reset_single_byte_bulk_command_sequence(painter_device_t device, const uint8_t *sequence, size_t sequence_len) {
    for (size_t i = 0; i < sequence_len;) {
        uint8_t command   = sequence[i];
        uint8_t delay     = sequence[i + 1];
        uint8_t num_bytes = sequence[i + 2];
        qp_comms_spi_dc_reset_single_byte_send_command(device, command);
        if (num_bytes > 0) {
            qp_comms_spi_dc_reset_single_byte_send_data(device, &sequence[i + 3], num_bytes);
        }
        if (delay > 0) {
            wait_ms(delay);
        }
        i += (3 + num_bytes);
    }
}
#    endif // QUANTUM_PAINTER_SPI_DC_RESET_ENABLE
#endif     // QUANTUM_PAINTER_SPI_ENABLE
