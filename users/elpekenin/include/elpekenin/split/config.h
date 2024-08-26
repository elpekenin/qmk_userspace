// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// needs to be here, for QMK to create the enum
#include "elpekenin/split/_ids.h"

#define SPLIT_WPM_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_LAYER_STATE_ENABLE

#define RPC_M2S_BUFFER_SIZE 80
#define RPC_S2M_BUFFER_SIZE 80
