// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/compiler_support.h>

#include "elpekenin/time.h"

STATIC_ASSERT(CM_ENABLED(UI), "Must enable 'elpekenin/ui'");
#include "elpekenin/ui.h"

static inline bool task_should_draw(uint32_t *last, uint32_t ms) {
    if (timer_elapsed32(*last) < ms) {
        return false;
    }

    *last = timer_read32();
    return true;
}
