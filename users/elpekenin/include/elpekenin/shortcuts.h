// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Handy aliases for repetitive and un-interesting code.
 */

// -- barrier --

#pragma once

/**
 * Convert a ``u16`` into 2x ``u8``
 */
#define U16_TO_U8(x) ((x) & 0xFF), ((x) >> 8)

/**
 * Get the ``n``'th bit out of ``val``.
 */
#define GET_BIT(val, n) (((val) >> (n)) & 1)

/**
 * Wrap some code such that it does not spit logging.
 *
 * .. note::
 *   Silences QMK messages, not (granted) `logging` ones.
 */
#define WITHOUT_DEBUG(code...)                          \
    do {                                                \
        bool old_debug_state     = debug_config.enable; \
        debug_config.enable      = false;               \
        code debug_config.enable = old_debug_state;     \
    } while (0)
