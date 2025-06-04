// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/assets.h"

#include <quantum/compiler_support.h>
#include <quantum/quantum.h>
#include <quantum/util.h>
#include <sys/cdefs.h>

STATIC_ASSERT(CM_ENABLED(GENERICS), "Must enable 'elpekenin/generics'");
STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");

#include "elpekenin/generics.h"
#include "elpekenin/logging.h"

typedef enum {
    EMPTY,
    DEVICE,
    FONT,
    IMAGE,
} asset_kind_t;

static const char *asset_names[] = {
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
    size_t devices;
    size_t fonts;
    size_t images;
} count = {0};

static asset_t assets[QP_ASSETS_SIZE] = {0};

static inline size_t total_count(void) {
    return count.devices + count.fonts + count.images;
}

static void set(asset_kind_t kind, const char *name, const void *ptr) {
    const size_t n = total_count();
    if (n >= QP_ASSETS_SIZE) {
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
        case EMPTY:
            __unreachable();

        case DEVICE:
            count.devices += 1;
            break;

        case FONT:
            count.fonts += 1;
            break;

        case IMAGE:
            count.images += 1;
            break;
    }
}

// NOTE: get_by_index(FONT, 3) is not 3rd element in pool, but the third *font* in pool
static const void *get_by_index(asset_kind_t kind, size_t index) {
    const char *const type = asset_names[kind];

    size_t counter = 0;

    bool filter(asset_t asset) {
        if (asset.kind == kind) {
            if (counter == index) {
                return true;
            }

            counter += 1;
        }

        return false;
    }

    const asset_t *const asset = find(assets, total_count(), filter);
    if (asset == NULL) {
        logging(LOG_ERROR, "(%s, %d): not found", type, index);
        return NULL;
    }

    logging(LOG_DEBUG, "(%s, %d): %p", type, index, asset->ptr);
    return asset->ptr;
}

static const void *get_by_name(asset_kind_t kind, const char *name) {
    const char *const type = asset_names[kind];

    bool filter(asset_t asset) {
        return asset.kind == kind && asset.name != NULL && strcmp(asset.name, name) == 0;
    }

    const asset_t *const asset = find(assets, total_count(), filter);
    if (asset == NULL) {
        logging(LOG_ERROR, "(%s, '%s'): not found", type, name);
        return NULL;
    }

    logging(LOG_DEBUG, "(%s, '%s'): %p", type, name, asset->ptr);
    return asset->ptr;
}

//

void qp_set_device_by_name(const char *name, painter_device_t device) {
    set(DEVICE, name, device);
}

painter_device_t qp_get_device_by_name(const char *name) {
    return get_by_name(DEVICE, name);
}

size_t qp_get_num_devices(void) {
    return count.devices;
}

painter_device_t qp_get_device_by_index(size_t index) {
    return get_by_index(DEVICE, index);
}

//

void qp_set_font_by_name(const char *name, const uint8_t *font) {
    set(FONT, name, font);
}

painter_font_handle_t qp_get_font_by_name(const char *name) {
    return get_by_name(FONT, name);
}

size_t qp_get_num_fonts(void) {
    return count.fonts;
}

painter_font_handle_t qp_get_font_by_index(size_t index) {
    return get_by_index(FONT, index);
}

//

void qp_set_image_by_name(const char *name, const uint8_t *image) {
    set(IMAGE, name, image);
}

painter_image_handle_t qp_get_image_by_name(const char *name) {
    return get_by_name(IMAGE, name);
}

size_t qp_get_num_images(void) {
    return count.images;
}

painter_image_handle_t qp_get_image_by_index(size_t index) {
    return get_by_index(IMAGE, index);
}
