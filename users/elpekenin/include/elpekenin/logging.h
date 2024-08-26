// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include <quantum/logging/print.h>

#include "backtrace.h"

#include "elpekenin/errno.h"
#include "elpekenin/utils/compiler.h"

// *** Debugging helpers ***  print on a single backend

NON_NULL(1) NON_NULL(2) READ_ONLY(1) void print_str(const char *str, const sendchar_func_t func);
NON_NULL(2) void print_u8(const uint8_t val, const sendchar_func_t func);
NON_NULL(1) NON_NULL(3) READ_ONLY(1) void print_u8_array(const uint8_t *list, const size_t len, const sendchar_func_t func);


// *** Custom logger *** (inspired by python's logging module)
typedef enum {
    UNKNOWN,
    LOGGER,
    QP,
    SCROLL,
    SIPO,
    SPLIT,
    SPI,
    TOUCH,
    MAP,
    ALLOC,
    __N_FEATURES__
} feature_t; // ALWAYS ADD AT THE END, FOR ASSERT TO WORK

typedef enum {
    LOG_NONE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    __N_LEVELS__
} log_level_t; // ALWAYS ADD AT THE END, FOR ASSERT TO WORK

PURE log_level_t get_level_for(feature_t feature);
void set_level_for(feature_t feature, log_level_t level);
void step_level_for(feature_t feature, bool increase);


// discard values the Python/zig way
extern int _;
NON_NULL(3) PRINTF(3) READ_ONLY(3) WARN_UNUSED int logging(feature_t feature, log_level_t level, const char *msg, ...);

// for sendchar backends to see the current message's level
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

// Specifiers supported:
    // %F - The feature's name (nothing if `UNKNOWN`)

    // %LL - The message's level (long)  | TRACE
    // %LS - The message's level (short) | T

    // %M - The actual message specified by `fmt` and `...` on `logging` - can contain regular specifiers

    // %T - Current time, you can override `char *log_time(void);` func to hook it with a RTC or whatever. Defaults to seconds since boot
    //    >>> snprintf(buff, sizeof(buff), "%d", timer_read32() / 1000);
    //    >>> return buff;
RETURN_NO_NULL const char *log_time(void);

    // %% - Write a "%"

// check length of the current format (ie: to make a buffer where to copy temporarily)
uint8_t get_logging_fmt_len(void);

// copy the current format
// NOTE: make sure the destinations is big enough
NON_NULL(1) WRITE_ONLY(1) void get_logging_fmt(char *dest);

// returns whether the format was valid (thus, got applied)
// NOTE: make sure the pointer is still valid (ie: not an automatic variable)
//       code doesn't make a copy of it, but just keep a pointer to its start
#ifndef MAX_LOG_FMT_LEN
#    define MAX_LOG_FMT_LEN (255)
#endif

NON_NULL(1) READ_ONLY(1) WARN_UNUSED int set_logging_fmt(const char *fmt);

#define ASSERT_FEATURES(__array) _Static_assert(ARRAY_SIZE(__array) == __N_FEATURES__, "Wrong size")
#define ASSERT_LEVELS(__array) _Static_assert(ARRAY_SIZE(__array) == __N_LEVELS__, "Wrong size")


void dump_stack(void);

#define log_success(success, feature, msg, args...) \
    do { \
        log_level_t level = success ? LOG_DEBUG : LOG_ERROR; \
        const char *out = (success) ? "OK" : "Error"; \
        _ = logging(feature, level, msg ": %s", ##args, out); \
    } while(0)
