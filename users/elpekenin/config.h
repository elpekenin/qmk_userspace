// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// Having this file on 'users/elpekenin/include' does not work, even though it is on path, there's probably something like:
// >>> #include "$(USER_PATH)/config.h"

#pragma once

#include "elpekenin/logging/config.h"
#include "elpekenin/qp/config.h"
#include "elpekenin/rgb/matrix/config.h"
#include "elpekenin/split/config.h"

#define MAX_DEFERRED_EXECUTORS 32

#define LAYER_STATE_8BIT
#define DYNAMIC_KEYMAP_LAYER_COUNT 5

#define UNICODE_SELECTED_MODES UNICODE_MODE_WINCOMPOSE
#define UCIS_MAX_CODE_POINTS 11
