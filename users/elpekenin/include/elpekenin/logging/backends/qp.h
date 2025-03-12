// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/quantum.h>

#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/compiler.h"

void qp_log_clear(void);
NON_NULL(1) READ_ONLY(1) void qp_logging_backend_render(qp_callback_args_t *args);
