// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// expose some macros to all files
//

#include "elpekenin/compiler.h"
#include "elpekenin/kconfig.h"

//
// utility macros
//

// similar to offsetof, but this doesn't exist
#define member_size(type, member) (sizeof(((type *)NULL)->member))

//
// community modules
//

// #define ALLOCATOR_DEBUG
// #define UI_DEBUG
#define ALLOC_ALLOCATORS_SIZE 5
#define ALLOC_ALLOCATIONS_SIZE 300
#define CRASH_UNWIND_DEPTH 20
#define CRASH_MESAGE_LENGTH 100
#define LOGGING_FORMAT "[%T] (%LS): %M"

//
// logging
//

#undef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1

//
// RGB
//

#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_LEFT_RIGHT

//
// split
//

// clang-format off
#define SPLIT_TRANSACTION_IDS_USER \
    RPC_ID_USER_LOGGING, \
    RPC_ID_USER_EEPROM_CLEAR, \
    RPC_ID_USER_XAP, \
    RPC_ID_USER_BUILD_ID, \
    ELPEKENIN_SYNC_ID
// clang-format on
