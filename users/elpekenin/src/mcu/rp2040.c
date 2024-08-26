// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ch.h>

#include "elpekenin/errno.h"
#include "elpekenin/logging.h"
#include "elpekenin/utils/sections.h"

volatile static bool ready = false;

static void start_signal(void) {
    ready = true;
}
PEKE_POST_INIT(start_signal, POST_INIT_CORE1);

static void stop_signal(bool unused) {
    ready = false;
}
PEKE_DEINIT(stop_signal, DEINIT_CORE1);


// will be a no-op if PICO_SDK_WRAPPERS is disabled on elpekenin/mk/mcu.mk
// these are defined by PicoSDK
extern init_fn __preinit_array_base__, __preinit_array_end__;

static void pico_sdk_init(void) {
    for (
        init_fn *func = &__preinit_array_base__;
        func < &__preinit_array_end__;
        func++
    ) {
        (*func)();
    }
}
PEKE_PRE_INIT(pico_sdk_init, INIT_SDK);


#if defined(SECOND_CORE_TASKS)
// core0
void __wrap_qp_internal_task(void) { }
void __wrap_deferred_exec_task(void) { }
void __wrap_housekeeping_task(void) { }

// core1
extern void __real_qp_internal_task(void);
extern void __real_deferred_exec_task(void);
extern void __real_housekeeping_task(void);

PEKE_CORE1_LOOP(__real_qp_internal_task);
PEKE_CORE1_LOOP(__real_deferred_exec_task);
PEKE_CORE1_LOOP(__real_housekeeping_task);
#endif

void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);

    chSysUnlock();

    // wait for everything to be configured
    while (!ready) { }

    _ = logging(UNKNOWN, LOG_DEBUG, "Hello from core 1");

    // PEKE_CORE1_INIT
    FOREACH_SECTION(init_fn, core1_init, func) {
        (*func)();
    }

    while (true) {
        // PEKE_CORE1_LOOP
        FOREACH_SECTION(init_fn, core1_loop, func) {
            (*func)();
        }
    }
}
