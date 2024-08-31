// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include <quantum/logging/print.h>

#include "backtrace.h"

#include "elpekenin/errno.h"
#include "elpekenin/utils/compiler.h"

/**
 * Custom logging utilities, inspired by Python's |logging|_ module.
 *
 * .. note::
 *   Under the hood, this is just a wrapper on top of :c:func:`printf`.
 *
 * .. hack for link with inline-code styling
 * .. |logging| replace:: ``logging``
 * .. _logging: https://docs.python.org/3/library/logging.html
 */

/**
 * Different features of the code to be logged.
 *
 * .. warning::
 *
 *    If you want to add a new one, it has to be the last element.
 */
typedef enum {
    /** Any code which is not on listed below. */
    UNKNOWN,
    /** Warnings/errors from the logger itself. */
    LOGGER,
    /** Anything related to screens. */
    QP,
    /** Custom scrolling text API for QP. */
    SCROLL,
    /** Serial In, Parallel Out shift registers driver. */
    SIPO,
    /** Custom communication over split. */
    SPLIT,
    /** Multi-SPI bus driver. */
    SPI,
    /** Touch screen driver. */
    TOUCH,
    /** Hash-map-like data structure. */
    MAP,
    /** Tracker of dynamic allocations. */
    ALLOC,
    __N_FEATURES__,
} feature_t; // ALWAYS ADD AT THE END, FOR ASSERT TO WORK

/**
 * Different level of severity. Used to filter out messsages.
 *
 * .. warning::
 *
 *    If you want to add a new one, it has to be the last element.
 */
typedef enum {
    /** Discard message. */
    LOG_NONE,
    /** For debugging. */
    LOG_DEBUG,
    /** Some status reporting. */
    LOG_INFO,
    /** Something -non critical- went wrong. */
    LOG_WARN,
    /** Unrecoverable. */
    LOG_ERROR,
    __N_LEVELS__,
} log_level_t; // ALWAYS ADD AT THE END, FOR ASSERT TO WORK

/**
 * Emit a logging message.
 *
 * Args:
 *     feature: Piece of code being logged.
 *     level: Severity of the message.
 *     msg: Format string for the message.
 *     ...: Variadic arguments to fill the specifiers in :c:var:`msg`.
 *
 * Return:
 *    Whether message could be emited. See :doc:`./errno` for details
 */
NON_NULL(3) PRINTF(3) READ_ONLY(3) WARN_UNUSED int logging(feature_t feature, log_level_t level, const char *msg, ...);

/**
 * .. note::
 *
 *   The :c:func:`logging` function will apply an extra transformation to your input, based on a custom format.
 *   Its specifiers being:
 *
 *   * ``%F``: The feature's name (nothing if ``UNKNOWN``).
 *   * ``%LL``: The message's level (long). Eg: ``DEBUG``.
 *
 *     * These strings are set in :c:var:`level_str`.
 *   * ``%LS``: Print only the first char of the previous string. Eg: ``D``.
 *   * ``%M``: The actual message created by ``msg`` and ``...`` on `logging` - can contain regular specifiers.
 *   * ``%T``: Current time, you can override :c:func:`log_time` to hook it with a RTC or whatever.
 *
 *     * Default implementation is seconds since boot.
 *   * ``%%``: Write a literal ``%``.
 *
 *   You can query the current format using :c:func:`get_logging_fmt` or change it with :c:func:`set_logging_fmt`.
 */

/**
 * Get the current level for a feature.
 * Messages with a lower severity are dropped.
 */
PURE log_level_t get_level_for(feature_t feature);

/** Change the level set to for a feature. */
void set_level_for(feature_t feature, log_level_t level);

/**
 * Increase (or decrease) by one the level set for a feature.
 * The direction is based on :c:var:`increase`.
 */
void step_level_for(feature_t feature, bool increase);

/**
 * Get length of the current logging format.
 * Use this to allocate a big-enough buffer before querying.
 */
uint8_t get_logging_fmt_len(void);

/**
 * Copy the current logging format into :c:var:`dest`.
 *
 * .. warning::
 *
 *   Destination must be big enough, use :c:func:`get_logging_fmt_len`.
 */
NON_NULL(1) WRITE_ONLY(1) void get_logging_fmt(char *dest);

/**
 * Change the logging format.
 *
 * Args:
 *     fmt: The new format, can't be longer than :c:macro:`MAX_LOG_FMT_LEN`.
 *
 * Return:
 *    Whether it could be set. See :doc:`./errno` for details.
 */
NON_NULL(1) READ_ONLY(1) WARN_UNUSED int set_logging_fmt(const char *fmt);


// discard values the Python/zig way
extern int _;

/**
 * .. warning::
 *
 *   From this point, the functions are mostly implementation details.
 *   You, most likely, don't need to know anything about them.
 */

/**
 * Get the severity of the message being emited.
 *
 * This may be used by a :c:type:`sendchar_func_t` internally.
 */
PURE log_level_t get_message_level(void);

// Change format used to print messages, can contain any text BUT \n
typedef enum {
    STR_END,
    NO_SPEC,
    INVALID_SPEC,
    F_SPEC,
    LL_SPEC,
    LS_SPEC,
    M_SPEC,
    PERC_SPEC,
    T_SPEC,
} token_t;

/**
 * Get a string representing the current time.
 * By default, seconds since boot, but it can be overwriten.
 */
RETURN_NO_NULL const char *log_time(void);

// returns whether the format was valid (thus, got applied)
// NOTE: make sure the pointer is still valid (ie: not an automatic variable)
//       code doesn't make a copy of it, but just keep a pointer to its start
#ifndef MAX_LOG_FMT_LEN
#    define MAX_LOG_FMT_LEN (255)
#endif

#define ASSERT_FEATURES(__array) _Static_assert(ARRAY_SIZE(__array) == __N_FEATURES__, "Wrong size")
#define ASSERT_LEVELS(__array) _Static_assert(ARRAY_SIZE(__array) == __N_LEVELS__, "Wrong size")


void dump_stack(void);

// *** Debugging helpers ***  print on a single backend

NON_NULL(1) NON_NULL(2) READ_ONLY(1) void print_str(const char *str, const sendchar_func_t func);
NON_NULL(2) void print_u8(const uint8_t val, const sendchar_func_t func);
NON_NULL(1) NON_NULL(3) READ_ONLY(1) void print_u8_array(const uint8_t *list, const size_t len, const sendchar_func_t func);

#define log_success(success, feature, msg, args...) \
    do { \
        log_level_t level = success ? LOG_DEBUG : LOG_ERROR; \
        const char *out = (success) ? "OK" : "Error"; \
        _ = logging(feature, level, msg ": %s", ##args, out); \
    } while(0)
