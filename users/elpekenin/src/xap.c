// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/xap.h"

#include <quantum/quantum.h>

static uint32_t xap_last_msg = 0;

uint32_t xap_last_activity_time(void) {
    return xap_last_msg;
}

uint32_t xap_last_activity_elapsed(void) {
    return timer_elapsed32(xap_last_msg);
}

void xap_last_activity_update(void) {
    xap_last_msg = timer_read32();
}
