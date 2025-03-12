// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

#include "elpekenin/utils/compiler.h"

typedef enum {
    NO_MOVEMENT, // only on start
    TOP,
    LEFT,
    BOTTOM,
    RIGHT,
} movement_t;

typedef struct PACKED {
    uint16_t x;
    uint16_t y;
} position_t;

void game_reset(void);

void game_frame(void);
void set_game_movement(movement_t direction);
