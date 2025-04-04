// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <quantum/color.h>

#include "elpekenin/logging.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/sipo.h"
// eInk is on left side, dont allocate framebuffer on right
#    if IS_LEFT_HAND
#        include "qp_eink_panel.h"
configure_sipo_pins(__PADDING__, IL91874_RST_PIN, IL91874_SRAM_CS_PIN, SCREENS_DC_PIN, IL91874_CS_PIN);

painter_device_t il91874;

uint8_t il91874_buffer[EINK_BYTES_REQD(IL91874_WIDTH, IL91874_HEIGHT)];
#    else
configure_sipo_pins(ILI9341_TOUCH_CS_PIN, SCREENS_DC_PIN, ILI9163_RST_PIN, ILI9163_CS_PIN, ILI9341_RST_PIN, ILI9341_CS_PIN);
painter_device_t ili9163;
painter_device_t ili9341;
#    endif
#endif

#if defined(QUANTUM_PAINTER_ENABLE) && defined(TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND

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
#endif

void keyboard_post_init_kb(void) {
    debug_config.enable = true;

    bool ret = true;
    (void)ret; // compiler warns "unused" when features are disabled

#if defined(QUANTUM_PAINTER_ENABLE)
    // SIPO
    gpio_set_pin_output(SIPO_CS_PIN);
    gpio_write_pin_high(SIPO_CS_PIN);

    // QP
    wait_ms(150); // Let screens draw some power

#    if IS_LEFT_HAND
    il91874 = qp_il91874_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, IL91874_CS_PIN, SCREENS_DC_PIN, IL91874_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE, (void *)il91874_buffer);
    ret &= qp_init(il91874, IL91874_ROTATION);
    ret &= qp_power(il91874, true);
#    endif

#    if IS_RIGHT_HAND
    ili9163 = qp_ili9163_make_spi_device(_ILI9163_WIDTH, _ILI9163_HEIGHT, ILI9163_CS_PIN, SCREENS_DC_PIN, ILI9163_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    ret &= qp_init(ili9163, ILI9163_ROTATION);
    ret &= qp_power(ili9163, true);

    ili9341 = qp_ili9341_make_spi_device(_ILI9341_WIDTH, _ILI9341_HEIGHT, ILI9341_CS_PIN, SCREENS_DC_PIN, ILI9341_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    ret &= qp_init(ili9341, ILI9341_ROTATION);
    ret &= qp_power(ili9341, true);

    qp_rect(ili9163, 0, 0, ILI9163_WIDTH, ILI9163_HEIGHT, HSV_BLACK, true);
    qp_rect(ili9341, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, HSV_BLACK, true);
#    endif

    if (!ret) {
        logging(LOG_ERROR, "QP setup");
    } else {
        logging(LOG_INFO, "QP setup");
    }
#endif

#if defined(QUANTUM_PAINTER_ENABLE) && defined(TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
    ret = touch_spi_init(ili9341_touch);

    if (!ret) {
        logging(LOG_ERROR, "Touch setup");
    } else {
        logging(LOG_INFO, "Touch setup");
    }
#endif

    keyboard_post_init_user();
}
