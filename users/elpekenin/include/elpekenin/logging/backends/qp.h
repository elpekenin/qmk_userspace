// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#include "elpekenin/qp/tasks/common.h" // qp_callback_args_t

void sendchar_qp_init(void);

int8_t sendchar_qp(uint8_t chr);

void qp_log_clear(void);
void qp_logging_backend_render(qp_callback_args_t *args);
