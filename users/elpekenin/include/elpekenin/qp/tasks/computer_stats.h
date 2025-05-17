// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/qp/tasks/common.h"

#ifndef COMPUTER_STATS_ARRAY_SIZE
#    define COMPUTER_STATS_ARRAY_SIZE 30
#endif

qp_callback_args_t *get_computer_stats_args(void);

void push_computer_stats(uint8_t cpu, uint8_t ram);
