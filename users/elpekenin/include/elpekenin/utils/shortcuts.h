// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Handy aliases for repetitive and un-interesting code.
 */

/**
 * ----
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
 * Wrap some code such that it doesnt spit logging.
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

/**
 * Wrap :c:macro:`WITHOUT_DEBUG`, silencing ``feature``'s messages too.
 */
#define WITHOUT_LOGGING(feature, code...)               \
    do {                                                \
        log_level_t old_level = get_level_for(feature); \
        set_level_for(feature, LOG_NONE);               \
        WITHOUT_DEBUG(code);                            \
        set_level_for(feature, old_level);              \
    } while (0)
