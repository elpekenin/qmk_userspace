// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

typedef enum : uint8_t {
    QWERTY,
    FN1,
    FN2,
    FN3,
    RST,
} layer_names_t;

const char *get_layer_name(layer_names_t layer);
