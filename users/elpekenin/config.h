// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/kconfig.h"

// logging
#undef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1

// RGB
#define ENABLE_RGB_MATRIX_SPLASH
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_LEFT_RIGHT

// split
#define SPLIT_TRANSACTION_IDS_USER RPC_ID_BUILD_INFO, RPC_ID_USER_SHUTDOWN, RPC_ID_USER_LOGGING, RPC_ID_USER_EE_CLR, RPC_ID_XAP
