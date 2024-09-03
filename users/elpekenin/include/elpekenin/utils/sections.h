// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Utilities to add elements into specific linker sections, and then iterate over those sections.
 *
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <stdbool.h>

#include <quantum/util.h>

#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/ld.h"

/* Start at 1000 because alphabetical order.
 * Does not take extra resouces, is the name of a linker section, not a value.
 * 1000 instead of 100 or 10 provides plenty of room to tweak order in the future.
 */

/*
 * Run RP's SDK initialization procedures as soon as posible.
 * Otherwise we will likely crash due to broken stdlib functions.
 */
#define INIT_SDK 1000

/* Set up allocation tracker, this *attempts* to detect memory leaks. */
#define INIT_ALLOC 1010

/* Set up buffers that allow logging onto screens. */
#define INIT_QP_LOG 1020
/* Set up custom :c:func:`sendchar` function, which :c:func:`printf` calls. */
#define INIT_SENDCHAR 1021 /* after QP logging has been init */
/* Configure a format string for :c:func:`logging`. */
#define INIT_LOG_FORMAT 1022
/* Set up zig-level code. */
#define INIT_ZIG 1023 /* early on, but with logging available */

/* Make a copy of crash data, so we dont lose the information. */
#define INIT_CRASH 1030
/* Set up the struct that holds which features are/not enabled. */
#define INIT_BUILD 1030 /* before split, so that we send the right thing over wire */

/* Set up structs that configure screens' drawing. */
#define INIT_QP_TASKS_ARGS 1040
/* Set up custom keylog feature. */
#define INIT_KEYLOG_MAP 1040
/* Set up custom RGB indicators feature. */
#define INIT_INDICATORS_MAP 1040

/* Set up custom split communications. */
#define INIT_SPLIT 1050
/* Set up QMK's trilayer feature. */
#define INIT_TRI_LAYER 1050
/* Enable QMK's autocorrect setup. */
#define INIT_AUTOCORRECT 1050

/* Set up games. */
#define INIT_GAME 1060

/* Flag second core that everything is set up. */
#define POST_INIT_CORE1 9990

/* Stopping second core is first on the list. */
#define DEINIT_CORE1 1000
/* Warn computer we shutting down/restarting. */
#define DEINIT_XAP 1010
/* Clean up screens. */
#define DEINIT_QP 1020
/* Run zig-level cleanup code. */
#define DEINIT_ZIG 1020
/* Stop lights as a marker that we aren't running. */
#define DEINIT_RGB 1020

/**
 * Signature for an initializer function.
 */
typedef void (*init_fn)(void);

/**
 * Signature for a finalizer function.
 */
typedef void (*deinit_fn)(bool jump_to_bootloader);

/**
 * Signature for a low-level function called by :c:func:`printf`.
 */
typedef int8_t (*sendchar_func_t)(uint8_t character);

/**
 * ----
 */

// -- barrier --

#define FULL_SECTION_NAME(name, func, prio) SECTION(STR(LD_NAME(name)) "." #prio "." #func)

/**
 * Put a function in the pre-initialization section.
 *
 * Args:
 *     func: The function to be added.
 *     prio: Its priority, lower number -> called earlier
 */
#define PEKE_PRE_INIT(func, prio) FULL_SECTION_NAME(pre_init, func, prio) USED static init_fn __##func = func

/**
 * Put a function in the post-initialization section.
 *
 * Args:
 *     func: The function to be added.
 *     prio: Its priority, lower number -> called earlier
 */
#define PEKE_POST_INIT(func, prio) FULL_SECTION_NAME(post_init, func, prio) USED static init_fn __##func = func

/**
 * Put a function in the second core's initialization section.
 *
 * Args:
 *     func: The function to be added.
 */
#define PEKE_CORE1_INIT(func) FULL_SECTION_NAME(core1_init, func, 0) USED static init_fn __##func = func

/**
 * Put a function in the second core's mainloop section.
 *
 * Args:
 *     func: The function to be added.
 */
#define PEKE_CORE1_LOOP(func) FULL_SECTION_NAME(core1_loop, func, 0) USED static init_fn __##func = func

/**
 * Put a function in the finalizer section.
 *
 * Args:
 *     func: The function to be added.
 *     prio: Its priority, lower number -> called earlier
 */
#define PEKE_DEINIT(func, prio) FULL_SECTION_NAME(deinit, func, prio) USED static deinit_fn __##func = func

/**
 * Put a function in the section that implements :c:func:`printf`'s logic.
 *
 * Args:
 *     func: The function to be added.
 */
#define PEKE_SENDCHAR(func) FULL_SECTION_NAME(sendchar, func, 0) USED static sendchar_func_t __##func = func

/**
 * ----
 */

/**
 * Macro to iterate all elements on a linker section.
 *
 * Args:
 *     type: Type of the elements in this section.
 *     name: Name of the section to be iterated.
 *     var: Name of the variable used on the ``for``
 */
#define FOREACH_SECTION(type, name, var)      \
    extern type LD_START(name), LD_END(name); \
    for (type *var = &LD_START(name); var < &LD_END(name); ++var)
