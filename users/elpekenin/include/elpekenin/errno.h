// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * .. _errno:
 *
 * Integers representing a function's status:
 *   * ``0``: Success.
 *   * Othewise: Failure.
 *
 * .. note::
 *
 *   Most values here come from ``#include <errno.h>``, take a look at Linux's documentation for details.
 */

// barrier for comment not to be attached to the include

#include <errno.h>

/** Element not found in map. */
#define ENOTFOUND 2404 // according to docstring on errno file, user should start at 2000
