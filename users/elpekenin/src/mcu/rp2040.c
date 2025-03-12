// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ch.h>

#include "elpekenin/dual_rp.h"
#include "elpekenin/logging.h"
#include "elpekenin/sections.h"

#if defined(SECOND_CORE_TASKS)
// core0
void __wrap_qp_internal_task(void) {}
void __wrap_deferred_exec_task(void) {}
void __wrap_housekeeping_task(void) {}

// core1
extern void __real_qp_internal_task(void);
extern void __real_deferred_exec_task(void);
extern void __real_housekeeping_task(void);

PEKE_CORE1_LOOP(__real_qp_internal_task);
PEKE_CORE1_LOOP(__real_deferred_exec_task);
PEKE_CORE1_LOOP(__real_housekeeping_task);
#endif

void c1_init_user(void) {
    logging(UNKNOWN, LOG_DEBUG, "Hello from core 1");

    // PEKE_CORE1_INIT
    FOREACH_SECTION(init_fn, core1_init, func) {
        (*func)();
    }
}

void c1_main_user(void) {
    // PEKE_CORE1_LOOP
    FOREACH_SECTION(init_fn, core1_loop, func) {
        (*func)();
    }
}
