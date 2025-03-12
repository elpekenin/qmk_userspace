// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include <quantum/quantum.h>

#include "version.h"

#include "elpekenin/build_info.h"
#include "elpekenin/utils/sections.h"

static build_info_t build_info = {0};

static void fill_build_info(void) {
    strlcpy(build_info.commit, QMK_GIT_HASH, sizeof(build_info.commit));
    build_info.features = get_enabled_features();
}
PEKE_PRE_INIT(fill_build_info, INIT_BUILD);

build_info_t get_build_info(void) {
    return build_info;
}

void set_build_info(build_info_t new_build_info) {
    memcpy(&build_info, &new_build_info, sizeof(build_info_t));
}
