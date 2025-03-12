// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/assets.h"

#include <quantum/quantum.h>

#include "elpekenin/dyn_array.h"
#include "elpekenin/logging.h"
#include "elpekenin/map.h"
#include "elpekenin/sections.h"
#include "generated/qp_resources.h"

static new_map(painter_device_t, device_map);
static new_map(painter_font_handle_t, font_map);
static new_map(painter_image_handle_t, image_map);

void qp_set_device_by_name(const char *name, painter_device_t display) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(device_map.values));
    map_set(device_map, name, display);
}

painter_device_t qp_get_device_by_name(const char *name) {
    int ret;

    painter_device_t device = map_get(device_map, name, ret);
    if (ret == 0) {
        return device;
    }

    return NULL;
}

uint8_t qp_get_num_displays(void) {
    return array_len(device_map.values);
}

painter_device_t qp_get_device_by_index(uint8_t index) {
    if (index >= qp_get_num_displays()) {
        return NULL;
    }

    return device_map.values[index];
}

//

void qp_set_font_by_name(const char *name, const uint8_t *font) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(font_map.values));
    map_set(font_map, name, qp_load_font_mem(font));
}

painter_font_handle_t qp_get_font_by_name(const char *name) {
    int ret;

    painter_font_handle_t font = map_get(font_map, name, ret);
    if (ret == 0) {
        return font;
    }

    return NULL;
}

uint8_t qp_get_num_fonts(void) {
    return array_len(font_map.values);
}

painter_font_handle_t qp_get_font_by_index(uint8_t index) {
    if (index >= qp_get_num_fonts()) {
        return NULL;
    }

    return font_map.values[index];
}

//

void qp_set_image_by_name(const char *name, const uint8_t *img) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(image_map.values));
    map_set(image_map, name, qp_load_image_mem(img));
}

painter_image_handle_t qp_get_image_by_name(const char *name) {
    int ret;

    painter_image_handle_t img = map_get(image_map, name, ret);
    if (ret == 0) {
        return img;
    }

    return NULL;
}

uint8_t qp_get_num_images(void) {
    return array_len(image_map.values);
}

painter_image_handle_t qp_get_image_by_index(uint8_t index) {
    if (index >= qp_get_num_images()) {
        return NULL;
    }

    return image_map.values[index];
}

// initialize maps
static uint8_t       qp_heap_buf[1000];
static memory_heap_t qp_heap;
static allocator_t   qp_allocator;

static void elpekenin_qp_maps_init(void) {
    chHeapObjectInit(&qp_heap, &qp_heap_buf, sizeof(qp_heap_buf));
    qp_allocator = new_ch_heap_allocator(&qp_heap, "qp heap");

    map_init(device_map, 2, &qp_allocator);
    map_init(font_map, 2, &qp_allocator);
    map_init(image_map, 10, &qp_allocator);

    load_qp_resources();
}
PEKE_PRE_INIT(elpekenin_qp_maps_init, INIT_QP_MAPS);
