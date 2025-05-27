// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>
#include <tmk_core/protocol/usb_descriptor.h>

#include "elpekenin/ring_buffer.h"

#define MAX_PAYLOAD_SIZE (XAP_EPSIZE - sizeof(xap_broadcast_header_t)) // -1 for terminator

static new_rbuf(char, XAP_LOG_BUFFER_SIZE, rbuf);

int8_t sendchar_xap(uint8_t chr) {
    rbuf_push(rbuf, chr);

    if (chr == '\n' || rbuf_full(rbuf)) {
        char   buff[MAX_PAYLOAD_SIZE];
        size_t size = rbuf_pop(rbuf, sizeof(buff) - 1, buff);
        buff[size]  = '\0';
        xap_broadcast(0x00, buff, size);
    }

    return 0;
}
