// Copyright 2025 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/utils/sections.h"

#include <lib/RTT/SEGGER_RTT.h>

static int8_t sendchar_rtt_hook(uint8_t c) {
#if ENABLE_RTT == 1
    return SEGGER_RTT_PutChar(0, c);
#else
    return 0;
#endif
}
PEKE_SENDCHAR(sendchar_rtt_hook);
