// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdbool.h>

#include "backtrace.h"

#include "elpekenin/utils/compiler.h"

/** Check if program crashed on previous execution. */
bool program_crashed(void);

/**
 * Get the information about last execution.
 *
 * Args:
 *     depth: Pointer to your variable, will be set to
 *            how deep the backtrace is.
 *
 * Returns:
 *     Call stack that crashed the program (NULL if didn't
 *     crash)
 */
NON_NULL(1) RETURN_NO_NULL WRITE_ONLY(1) backtrace_t *get_crash_call_stack(uint8_t *depth);

/** Tiny utility to print the information about last crash. */
void print_crash_call_stack(void);

/**
 * Clear the structure that holds crash information.
 * So that we dont mistakenly think, on next iteration,
 * that current one crashed, because it was left there.
 */
void clear_crash_info(void);

/**
 * Store the information that next run will read.
 *
 * Args:
 *     msg: A text that will be stored along backtrace.
 */
void set_crash_info(const char *msg);
