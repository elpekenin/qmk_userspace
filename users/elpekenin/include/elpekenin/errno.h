// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * .. _errno:
 *
 * Integers representing a function's status:
 *   * ``0``: Success.
 *   * Othewise: Failure.
 *
 * .. seealso::
 *   Most values here come from ``#include <errno.h>``.
 *
 *   Check out Linux's documentation for details.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include <errno.h>

/**
 * Element not found in map.
 */
#define ENOTFOUND 2404 // according to docstring on errno file, user should start at 2000
