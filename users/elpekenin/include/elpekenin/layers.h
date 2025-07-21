// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/compiler_support.h>
#include <stdint.h>

typedef enum {
    QWERTY,
    FN1,
    FN2,
    FN3,
    RST,
} layer_t;
STATIC_ASSERT(~(layer_t)0 <= UINT8_MAX, "layer_t expected to be 8bit");

const char *get_layer_name(layer_t layer);
