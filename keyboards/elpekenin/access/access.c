// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <quantum/color.h>

#include "elpekenin/sipo.h"

// compat: include-dir-dependant #include fails
#if IS_ENABLED(QUANTUM_PAINTER)
#    include <drivers/painter/eink_panel/qp_eink_panel.h>
#else
#    define EINK_BYTES_REQD(x, y) (1)
#endif

extern painter_device_t qp_ili9163_make_sipo_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode);

extern painter_device_t qp_ili9341_make_sipo_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode);

enum left_sipo_pins {
    // there's an unused pin
    IL91874_RST_PIN = 1,
    IL91874_SRAM_CS_PIN,
    SCREENS_DC_PIN_LEFT,
    IL91874_CS_PIN,
};

enum right_sipo_pins {
    ILI9341_TOUCH_CS_PIN,
    SCREENS_DC_PIN_RIGHT,
    ILI9163_RST_PIN,
    ILI9163_CS_PIN,
    ILI9341_RST_PIN,
    ILI9341_CS_PIN,
};

uint8_t il91874_buffer[EINK_BYTES_REQD(IL91874_WIDTH, IL91874_HEIGHT)] = {0};

painter_device_t il91874 = {0};
painter_device_t ili9163 = {0};
painter_device_t ili9341 = {0};

// Calibration isn't very precise
static const touch_driver_t ili9341_touch_driver = {
    .width       = _ILI9341_WIDTH,
    .height      = _ILI9341_HEIGHT,
    .scale_x     = 0.07,
    .scale_y     = -0.09,
    .offset_x    = -26,
    .offset_y    = 345,
    .rotation    = (ILI9341_ROTATION + 2) % 4,
    .upside_down = false,
    .spi_config =
        {
            .chip_select_pin = ILI9341_TOUCH_CS_PIN,
            .divisor         = TOUCH_SPI_DIV,
            .lsb_first       = false,
            .mode            = TOUCH_SPI_MODE,
            .irq_pin         = NO_PIN,
            .x_cmd           = 0xD0,
            .y_cmd           = 0x90,
        },
};
touch_device_t ili9341_touch = &ili9341_touch_driver;

void keyboard_post_init_kb(void) {
    debug_config.enable = true;

    if (IS_ENABLED(QUANTUM_PAINTER)) {
        gpio_set_pin_output(SIPO_CS_PIN);
        gpio_write_pin_high(SIPO_CS_PIN);

        wait_ms(150); // Let screens draw some power
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && is_keyboard_left()) {
        bool ret = true;

        // compat: factory function not available
#if IS_ENABLED(QUANTUM_PAINTER_IL91874_SPI)
        // TODO: see `qp/sipo.c`
        il91874 = qp_il91874_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, IL91874_CS_PIN, SCREENS_DC_PIN_LEFT, IL91874_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE, (void *)il91874_buffer);
#endif
        ret &= qp_init(il91874, IL91874_ROTATION);
        ret &= qp_power(il91874, true);

        printf("QP setup: %s\n", ret ? "ok" : "failed");
    }

    if (IS_ENABLED(QUANTUM_PAINTER) && !is_keyboard_left()) {
        bool ret = true;

#if IS_ENABLED(QUANTUM_PAINTER_ILI9163_SPI)
        ili9163 = qp_ili9163_make_sipo_device(_ILI9163_WIDTH, _ILI9163_HEIGHT, ILI9163_CS_PIN, SCREENS_DC_PIN_RIGHT, ILI9163_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
#endif
        ret &= qp_init(ili9163, ILI9163_ROTATION);
        ret &= qp_power(ili9163, true);
        qp_rect(ili9163, 0, 0, ILI9163_WIDTH, ILI9163_HEIGHT, HSV_BLACK, true);

#if IS_ENABLED(QUANTUM_PAINTER_ILI9341_SPI)
        ili9341 = qp_ili9341_make_sipo_device(_ILI9341_WIDTH, _ILI9341_HEIGHT, ILI9341_CS_PIN, SCREENS_DC_PIN_RIGHT, ILI9341_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
#endif
        ret &= qp_init(ili9341, ILI9341_ROTATION);
        ret &= qp_power(ili9341, true);
        qp_rect(ili9341, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, HSV_BLACK, true);

        printf("QP setup: %s\n", ret ? "ok" : "failed");
    }

    if (IS_ENABLED(TOUCH_SCREEN) && !is_keyboard_left()) {
        bool ret = touch_spi_init(ili9341_touch);
        printf("Touch setup: %s\n", ret ? "ok" : "failed");
    }

    keyboard_post_init_user();
}
