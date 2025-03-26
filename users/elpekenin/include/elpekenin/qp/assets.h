// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

void qp_assets_init(void);

//

void qp_set_device_by_name(const char *name, painter_device_t display);

painter_device_t qp_get_device_by_name(const char *name);

uint8_t qp_get_num_displays(void);

painter_device_t qp_get_device_by_index(uint8_t index);

//

void qp_set_font_by_name(const char *name, const uint8_t *font);

painter_font_handle_t qp_get_font_by_name(const char *name);

uint8_t qp_get_num_fonts(void);

painter_font_handle_t qp_get_font_by_index(uint8_t index);

//

void qp_set_image_by_name(const char *name, const uint8_t *img);

painter_image_handle_t qp_get_image_by_index(uint8_t index);

uint8_t qp_get_num_images(void);

painter_image_handle_t qp_get_image_by_name(const char *name);
