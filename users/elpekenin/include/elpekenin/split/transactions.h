// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/compiler_support.h>
#include <quantum/quantum.h>
#include <quantum/split_common/transactions.h>
#include <quantum/util.h>
#include <tmk_core/protocol/usb_descriptor.h> // XAP_EPSIZE

STATIC_ASSERT(CM_ENABLED(BUILD_ID), "Must enable 'elpekenin/build_id'");

#include "elpekenin/build_id.h"

void transactions_init(void);

void reset_ee_slave(void);

#if IS_ENABLED(XAP) || defined(__SPHINX__)
void xap_execute_slave(const void *data);

typedef struct PACKED {
    uint8_t data[XAP_EPSIZE];
} xap_split_msg_t;
STATIC_ASSERT(sizeof(xap_split_msg_t) == XAP_EPSIZE, "wrong size for xap_split_msg_t");
#endif

bool get_slave_build_id(u128 *build_id);
