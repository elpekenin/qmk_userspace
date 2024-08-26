// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO: Support for different games
#include "elpekenin/game.h"
#include "elpekenin/games/snake.h"
#include "elpekenin/utils/sections.h"

static game_state_t   game       = {0};
static deferred_token game_token = INVALID_DEFERRED_TOKEN;

static uint32_t game_callback(uint32_t trigger_time, void *cb_arg) {
    advance_snake_game(&game);
    return game.delay;
}

void set_game_device(painter_device_t device) {
    game.device = device;

    // dont want multiple "workers" at the same time
    cancel_deferred_exec(game_token);
    game_token = defer_exec(10, game_callback, NULL);
}


static void game_init(void) {
    game = new_snake_game();
}
PEKE_PRE_INIT(game_init, INIT_GAME);

// reset is just re-init
void game_reset(void) {
    game_init();
}

void set_game_movement(movement_t direction) {
    game.direction = direction;
}
