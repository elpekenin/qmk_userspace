// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/util.h>

#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/ld.h"

/* Start at 1000 because alphabetical order.
 * Does not take extra resouces, is the name of a linker section, not a value.
 * 1000 instead of 100 or 10 provides plenty of room to tweak order in the future.
 */

/* SDK (RP-specific) setup has to be setup as soon as posible, so stdlib does not break
 * set up dynamic allocators usage tracker, before anything tries and allocate
 * next up, logging, so we can print stuff right after
 * copy information about crash before we potentially mess it up/overwrite
 * store build version information, before splít sends it across the wire
 * init some QMK stuff
 * lastly fill in maps
 */
#define INIT_SDK 1000

#define INIT_ALLOC 1010

#define INIT_QP_LOG     1020
#define INIT_SENDCHAR   1021 /* after QP logging has been init */
#define INIT_LOG_FORMAT 1022
#define INIT_ZIG        1023 /* early on, but with logging available */

#define INIT_CRASH 1030
#define INIT_BUILD 1030 /* before split, so that we send the right thing over wire */

#define INIT_QP_TASKS_ARGS  1040
#define INIT_KEYLOG_MAP     1040
#define INIT_INDICATORS_MAP 1040

#define INIT_SPLIT       1050
#define INIT_TRI_LAYER   1050
#define INIT_AUTOCORRECT 1050

#define INIT_GAME 1060


/* CORE1 has to be kept last, as it is used to tell second RP core that everything is ready
 */
#define POST_INIT_CORE1 9990


/* This order should not matter, but lets first "warn" slave
 * then XAP client, then shutdown screens and LEDs
 */
#define DEINIT_CORE1 1000

#define DEINIT_XAP 1010

#define DEINIT_QP  1020
#define DEINIT_ZIG 1020
#define DEINIT_RGB 1020


/* Function types
 */
typedef void   (*init_fn)(void);
typedef void   (*deinit_fn)(bool);
typedef int8_t (*sendchar_func_t)(uint8_t c);


/* Magic macros
 */
#define FULL_SECTION_NAME(name, func, prio) SECTION(STR(LD_NAME(name)) "." #prio "." #func)

#define FOREACH_SECTION(type, name, var) \
    extern type LD_START(name), LD_END(name); \
    for (type *var = &LD_START(name); var < &LD_END(name); ++var)

#define PEKE_PRE_INIT(func, prio) \
    FULL_SECTION_NAME(pre_init, func, prio) USED static init_fn __##func = func

#define PEKE_POST_INIT(func, prio) \
    FULL_SECTION_NAME(post_init, func, prio) USED static init_fn __##func = func

#define PEKE_CORE1_INIT(func) \
    FULL_SECTION_NAME(core1_init, func, 0) USED static init_fn __##func = func

#define PEKE_CORE1_LOOP(func) \
    FULL_SECTION_NAME(core1_loop, func, 0) USED static init_fn __##func = func

#define PEKE_DEINIT(func, prio) \
    FULL_SECTION_NAME(deinit, func, prio) USED static deinit_fn __##func = func

#define PEKE_SENDCHAR(func) \
    FULL_SECTION_NAME(sendchar, func, 0) USED static sendchar_func_t __##func = func
