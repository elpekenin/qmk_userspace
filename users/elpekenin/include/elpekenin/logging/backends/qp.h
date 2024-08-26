// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/painter/qp.h>

#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/compiler.h"

#if !defined(LOG_N_LINES)
#    define LOG_N_LINES 9
#endif // !defined(LOG_N_LINES)

#if !defined(LOG_N_CHARS)
#    define LOG_N_CHARS 70
#endif // !defined(LOG_N_CHARS)

#undef QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS
#define QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS (LOG_N_LINES + 5)

void qp_log_clear(void);
NON_NULL(1) READ_ONLY(1) void qp_logging_backend_render(qp_callback_args_t *args);
