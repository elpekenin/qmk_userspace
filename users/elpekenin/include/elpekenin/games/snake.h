// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/game.h"

#define TILE_SIZE 10
#define GAME_ROWS 10
#define GAME_COLS 10

typedef struct PACKED {
    bool             playing;
    movement_t       direction;
    painter_device_t device;

    // whether end of map wraps around or kills
    bool death_walls;

    // list where to find the "order" of body parts
    // useful to identify head/tail + do rotated sprites in the future
    uint8_t    snake_len;
    position_t snake[100];

    position_t food;

    uint32_t delay; // between frames
} game_state_t;

game_state_t new_snake_game(void);

NON_NULL(1) void advance_snake_game(game_state_t *state);
