// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/assets.h"

#include <quantum/quantum.h>
#include <quantum/util.h>
#include <sys/cdefs.h>

#include "elpekenin/logging.h"
#include "generated/qp_resources.h"

typedef enum {
    EMPTY,
    DEVICE,
    FONT,
    IMAGE,
} asset_kind_t;

const char *asset_names[] = {
    [EMPTY]  = "empty",
    [DEVICE] = "device",
    [FONT]   = "font",
    [IMAGE]  = "image",
};

typedef struct PACKED {
    asset_kind_t kind;
    const char  *name;
    const void  *ptr;
} asset_t;

static struct {
    uint8_t devices;
    uint8_t fonts;
    uint8_t images;
} count = {0};

static asset_t assets[CONFIG_QP_ASSETS_SIZE] = {
    [0 ... CONFIG_QP_ASSETS_SIZE - 1] =
        {
            .kind = EMPTY,
            .name = NULL,
            .ptr  = NULL,
        },
};

static void set(asset_kind_t kind, const char *name, const void *ptr) {
    const uint16_t n = count.devices + count.fonts + count.images;
    if (n >= CONFIG_QP_ASSETS_SIZE) {
        logging(LOG_ERROR, "%s: too many assets", __func__);
        return;
    }

    assets[n] = (asset_t){
        .kind = kind,
        .name = name,
        .ptr  = ptr,
    };

    const char *asset_name = asset_names[kind];
    logging(LOG_DEBUG, "Stored %s '%s'(%p) at index %d of assets pool", asset_name, name, ptr, n);

    switch (kind) {
        case DEVICE:
            count.devices += 1;
            break;

        case FONT:
            count.fonts += 1;
            break;

        case IMAGE:
            count.images += 1;
            break;

        default:
            __unreachable();
    }
}

// (FONT, 3) is not 3rd element in pool, but the third *font* in pool
static const void *get_by_index(asset_kind_t kind, uint8_t index) {
    const uint16_t n = count.devices + count.fonts + count.images;

    uint8_t counter = 0;
    for (uint16_t i = 0; i < n; ++i) {
        asset_t asset = assets[i];

        if (asset.kind == kind) {
            if (counter == index) {
                const char *asset_name = asset_names[kind];
                logging(LOG_DEBUG, "Read %s with index %d: (%p)", asset_name, n, asset.ptr);

                return asset.ptr;
            }

            counter += 1;
        }
    }

    return NULL;
}

static const void *get_by_name(asset_kind_t kind, const char *name) {
    const uint16_t n = count.devices + count.fonts + count.images;

    for (uint16_t i = 0; i < n; ++i) {
        asset_t asset = assets[i];

        if (asset.kind == kind && asset.name != NULL && strcmp(asset.name, name) == 0) {
            const char *asset_name = asset_names[kind];
            logging(LOG_DEBUG, "Read %s '%s': %p", asset_name, name, asset.ptr);

            return asset.ptr;
        }
    }

    return NULL;
}

//

void qp_set_device_by_name(const char *name, painter_device_t device) {
    set(DEVICE, name, device);
}

painter_device_t qp_get_device_by_name(const char *name) {
    return get_by_name(DEVICE, name);
}

uint8_t qp_get_num_devices(void) {
    return count.devices;
}

painter_device_t qp_get_device_by_index(uint8_t index) {
    return get_by_index(DEVICE, index);
}

//

void qp_set_font_by_name(const char *name, const uint8_t *font) {
    set(FONT, name, font);
}

painter_font_handle_t qp_get_font_by_name(const char *name) {
    return get_by_name(FONT, name);
}

uint8_t qp_get_num_fonts(void) {
    return count.fonts;
}

painter_font_handle_t qp_get_font_by_index(uint8_t index) {
    return get_by_index(FONT, index);
}

//

void qp_set_image_by_name(const char *name, const uint8_t *image) {
    set(IMAGE, name, image);
}

painter_image_handle_t qp_get_image_by_name(const char *name) {
    return get_by_name(IMAGE, name);
}

uint8_t qp_get_num_images(void) {
    return count.images;
}

painter_image_handle_t qp_get_image_by_index(uint8_t index) {
    return get_by_index(IMAGE, index);
}
