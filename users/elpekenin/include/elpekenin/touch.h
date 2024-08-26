// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

typedef const void * touch_device_t;

typedef struct PACKED {
    uint16_t x;
    uint16_t y;
    bool     pressed;
} touch_report_t;

typedef enum { TOUCH_ROTATION_0, TOUCH_ROTATION_90, TOUCH_ROTATION_180, TOUCH_ROTATION_270} touch_rotation_t;

typedef struct PACKED {
    pin_t    chip_select_pin;
    uint16_t divisor;
    bool     lsb_first;
    uint8_t  mode;
    pin_t    irq_pin;
    uint8_t  x_cmd;
    uint8_t  y_cmd;
} spi_touch_comms_config_t;

typedef struct PACKED {
    uint16_t width;
    uint16_t height;
    float scale_x;
    float scale_y;
    int16_t offset_x;
    int16_t offset_y;
    touch_rotation_t rotation;
    bool upside_down;
    spi_touch_comms_config_t spi_config;
} touch_driver_t;


bool touch_spi_init(touch_device_t device);
void report_from(int16_t x, int16_t y, touch_driver_t *driver, touch_report_t *report);
touch_report_t get_spi_touch_report(touch_device_t device, bool check_irq);
