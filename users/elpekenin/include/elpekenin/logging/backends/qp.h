// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include "elpekenin/qp/graphics.h" // qp_callback_args_t

int8_t sendchar_qp_hook(uint8_t c);

void qp_log_clear(void);
void qp_logging_backend_render(qp_callback_args_t *args);
