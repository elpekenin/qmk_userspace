// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

// these operate inplace and return a pointer to the input for convenience

NON_NULL(2) RETURN_NO_NULL WRITE_ONLY(2, 3) const char *pretty_bytes(size_t n, char *buffer, uint16_t buffer_size);

CONST bool is_utf8(char c);
CONST bool is_utf8_continuation(char c);