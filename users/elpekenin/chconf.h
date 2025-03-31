// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define CH_CFG_USE_HEAP TRUE
#define CH_CFG_USE_MEMPOOLS TRUE

#if defined(MCU_RP)
#    define CH_CFG_USE_MUTEXES TRUE
#endif

#include_next <chconf.h>
