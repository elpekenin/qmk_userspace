// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>
#include <tmk_core/protocol/usb_descriptor.h>

#if CM_ENABLED(TYPES)
#    include "elpekenin/types.h"
#else
#    error Must enable 'elpekenin/types'
#endif

#define MAX_PAYLOAD_SIZE (XAP_EPSIZE - sizeof(xap_broadcast_header_t)) // -1 for terminator

OptionImpl(char);
RingBufferImpl(char);

static char rbuf_inner[XAP_LOG_BUFFER_SIZE] = {0};

static RingBuffer(char) rbuf = rbuf_from(char, rbuf_inner);

int8_t sendchar_xap(uint8_t chr) {
    const bool pushed = rbuf.push(&rbuf, chr);

    // buffered
    if (chr != '\n' && pushed) {
        return 0;
    }

    // send
    char xap_buff[MAX_PAYLOAD_SIZE] = {0};
    for (size_t i = 0; i < MAX_PAYLOAD_SIZE - 1; ++i) {
        const Option(char) pop = rbuf.pop(&rbuf);
        if (!pop.is_some) {
            break;
        }

        xap_buff[i]     = unwrap(pop);
        xap_buff[i + 1] = '\0';
    }
    xap_broadcast(0x00, xap_buff, MAX_PAYLOAD_SIZE);

    // failed to push before, do it now that buffer was sent
    if (!pushed) {
        assert(rbuf.push(&rbuf, chr) == true);
    }

    return 0;
}
