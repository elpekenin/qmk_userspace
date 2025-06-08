// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/build_id.h"

typedef struct PACKED {
    u128 build_id;
} user_data_t;
STATIC_ASSERT(sizeof(user_data_t) <= EECONFIG_USER_DATA_SIZE, "Data won't fit");
