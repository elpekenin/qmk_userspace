// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/quantum.h>

#include "elpekenin/logging.h"
#include "elpekenin/ring_buffer.h"
#include "elpekenin/sections.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/string.h"

// *** Split data ***

typedef struct PACKED {
    bool    flush;
    uint8_t bytes;
} split_logging_header_t;

typedef struct PACKED {
    split_logging_header_t header;

    char buff[RPC_S2M_BUFFER_SIZE - sizeof(split_logging_header_t)];
} split_logging_t;
_Static_assert(sizeof(split_logging_t) == RPC_S2M_BUFFER_SIZE, "Wrong size");

// *** Slave ***

static new_rbuf(char, 200, slave_rbuf);

static int8_t sendchar_split_hook(uint8_t c) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return 0;
    }

    if (c >= '\n'      // terminator/non-printable chars
        && !is_utf8(c) // only accept ASCII
    ) {
        rbuf_push(slave_rbuf, c);
    }

    return 0;
}
PEKE_SENDCHAR(sendchar_split_hook);

void user_logging_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    split_logging_t data = {0};

    size_t size = rbuf_pop(slave_rbuf, ARRAY_SIZE(data.buff), data.buff);

    data.header.bytes = size;

    // work out the data
    for (size_t i = 0; i < size; ++i) {
        // update message: do we have to flush?
        if (data.buff[i] == '\n') {
            data.header.flush = true;
            break;
        }
    }

    // copy data on send buffer
    memcpy(s2m_buffer, &data, sizeof(split_logging_t));
}

// *** Master ***

static new_rbuf(char, 200, master_rbuf);

void user_logging_master_poll(void) {
    split_logging_t data = {0};
    transaction_rpc_recv(RPC_ID_USER_LOGGING, sizeof(data), &data);

    size_t size = data.header.bytes;
    if (size == 0) {
        return;
    }

    // copy received
    for (size_t i = 0; i < size; ++i) {
        rbuf_push(master_rbuf, data.buff[i]);
    }

    // flush if asked to
    if (data.header.flush || rbuf_full(master_rbuf)) {
        char   buff[100];
        size_t size = rbuf_pop(master_rbuf, sizeof(buff), buff);
        buff[size]  = '\0';

        // keep retrying until we get to write it
        while (true) {
            if (logging(UNKNOWN, LOG_DEBUG, "*****\n%s*****\n", buff) == 0) {
                break;
            }
        }
    }
}
