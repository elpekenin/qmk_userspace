// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/painter/qp.h>

//

void set_device_by_name(const char *name, painter_device_t device);

painter_device_t get_device_by_name(const char *name);

size_t get_num_devices(void);

painter_device_t get_device_by_index(size_t index);

//

void set_font_by_name(const char *name, const uint8_t *font);

painter_font_handle_t get_font_by_name(const char *name);

size_t get_num_fonts(void);

painter_font_handle_t get_font_by_index(size_t index);

//

void set_image_by_name(const char *name, const uint8_t *image);

painter_image_handle_t get_image_by_name(const char *name);

size_t get_num_images(void);

painter_image_handle_t get_image_by_index(size_t index);
