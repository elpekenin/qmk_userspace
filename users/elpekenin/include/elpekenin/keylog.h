// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/action.h> // keyrecord_t

#include "elpekenin/utils/compiler.h"

NON_NULL(1) void keycode_repr(const char **str);
NON_NULL(2) READ_ONLY(2) void keylog_process(uint16_t keycode, keyrecord_t *record);
PURE bool is_keylog_dirty(void);
RETURN_NO_NULL const char *get_keylog(void);