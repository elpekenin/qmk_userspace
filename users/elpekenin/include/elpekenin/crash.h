// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "backtrace.h"

#include "elpekenin/utils/compiler.h"

bool program_crashed(void);
NON_NULL(1) RETURN_NO_NULL WRITE_ONLY(1) backtrace_t *get_crash_call_stack(uint8_t *depth);
void print_crash_call_stack(void);
void clear_crash_info(void);

void set_crash_info(const char *msg);
