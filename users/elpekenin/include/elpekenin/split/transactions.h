// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/compiler_support.h>
#include <quantum/quantum.h>
#include <quantum/split_common/transactions.h>
#include <quantum/util.h>
#include <tmk_core/protocol/usb_descriptor.h> // XAP_EPSIZE

void transactions_init(void);

void reset_ee_slave(void);
void build_info_sync_keymap_callback(void);

void xap_execute_slave(const void *data);

typedef struct PACKED {
    uint8_t data[XAP_EPSIZE];
} xap_split_msg_t;
STATIC_ASSERT(sizeof(xap_split_msg_t) == XAP_EPSIZE, "wrong size for xap_split_msg_t");
