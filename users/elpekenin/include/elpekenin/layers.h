// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

typedef enum {
    _QWERTY,
    _FN1,
    _FN2,
    _FN3,
    _RST
} layer_names_t;

const char *get_layer_name(layer_names_t layer);
