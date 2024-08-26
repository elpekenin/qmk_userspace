// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

#include "generated/features.h"


#define COMMIT_TEXT_LEN 12
typedef struct PACKED {
    char               commit[COMMIT_TEXT_LEN];
    enabled_features_t features;
} build_info_t;

build_info_t get_build_info(void);

void set_build_info(build_info_t new_build_info);
