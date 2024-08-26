// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once 

#include "elpekenin/utils/compiler.h"

CONST READ_ONLY(1) bool ptr_in_heap(const void *addr);

CONST READ_ONLY(1) bool ptr_in_main_stack(const void *addr);
CONST READ_ONLY(1) bool ptr_in_process_stack(const void *addr);
CONST READ_ONLY(1) bool ptr_in_stack(const void *addr);

CONST size_t get_heap_size(void);

#if defined(MCU_RP)
CONST size_t get_used_flash(void);
CONST size_t get_flash_size(void);
#endif
