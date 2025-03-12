// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

#include "elpekenin/utils/compiler.h"

void user_logging_master_poll(void);

NON_NULL(2) NON_NULL(4) WRITE_ONLY(4) void user_logging_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer);
