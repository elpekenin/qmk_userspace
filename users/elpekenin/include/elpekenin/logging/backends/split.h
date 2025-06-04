// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

int8_t sendchar_split(uint8_t chr);

uint32_t user_logging_master_poll(void);

void logging_handler(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer);
