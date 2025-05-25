// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/kconfig.h"

// check for features at C (not pre-processor) level
// based on https://hmijailblog.blogspot.com/2016/03/an-isdefined-c-macro-to-check-whether.html
//
// this is useful so that dead code is still analyzed by compiler and detect bugs (pre-processor would "comment" instead)
//
// WARN: relies on the macro's value (when present) having a different stringified name than the macro's name
//
// NOTE: does not incur extra binary size because conditionals like `if (IS_DEFINED(FEATURE)) { feature_function() };`
//       will be stripped away thanks to compiler/linker identifying the function will never be used
//       otherwise we'd get an 'undefined symbol' error when trying to resolve the function name (it was not compiled)
#define STRINGIFY(x) "" #x
#define IS_DEFINED(MACRO) (sizeof("" #MACRO) != sizeof(STRINGIFY(MACRO)))

// misc
#define MAX_DEFERRED_EXECUTORS 64
#define LAYER_STATE_8BIT
#define DYNAMIC_KEYMAP_LAYER_COUNT 5

// logging
#undef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1

// QP
#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 0

// RGB
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SPLASH
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#define RGB_MATRIX_DEFAULT_VAL 30
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 150
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_LEFT_RIGHT

// split
#define SPLIT_TRANSACTION_IDS_USER RPC_ID_BUILD_INFO, RPC_ID_USER_SHUTDOWN, RPC_ID_USER_LOGGING, RPC_ID_USER_EE_CLR, RPC_ID_XAP
#define SPLIT_WPM_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_LAYER_STATE_ENABLE
#define RPC_M2S_BUFFER_SIZE 80
#define RPC_S2M_BUFFER_SIZE 80
