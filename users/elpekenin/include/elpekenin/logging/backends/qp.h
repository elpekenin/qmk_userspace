// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include "elpekenin/qp/graphics.h" // qp_callback_args_t

#ifndef LOG_N_LINES
#    define LOG_N_LINES 9
#endif

#ifndef LOG_N_CHARS
#    define LOG_N_CHARS 70
#endif

void sendchar_qp_init(void);

int8_t sendchar_qp(uint8_t c);

void qp_log_clear(void);
void qp_logging_backend_render(qp_callback_args_t *args);
