// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ctype.h>
#include <quantum/compiler_support.h>
#include <quantum/quantum.h>
#include <quantum/split_common/transactions.h>
#include <quantum/util.h>

#include "elpekenin/logging.h"
#include "elpekenin/ring_buffer.h"

typedef struct PACKED {
    bool    flush;
    uint8_t bytes;
} split_logging_header_t;

#define PAYLOAD_SIZE (RPC_S2M_BUFFER_SIZE - sizeof(split_logging_header_t))

typedef struct PACKED {
    split_logging_header_t header;
    char                   buff[PAYLOAD_SIZE];
} split_logging_t;
STATIC_ASSERT(sizeof(split_logging_t) == RPC_S2M_BUFFER_SIZE, "Wrong size");

// slave will write on its copy of this variable, master will copy (over split) onto its own
static new_rbuf(char, SPLIT_LOG_BUFFER_SIZE, ring_buffer);

int8_t sendchar_split(uint8_t chr) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return 0;
    }

    if (isprint(chr) || chr == '\n') {
        rbuf_push(ring_buffer, chr);
    }

    return 0;
}

void user_logging_slave_callback(__unused uint8_t m2s_size, __unused const void* m2s_buffer, __unused uint8_t s2m_size, void* s2m_buffer) {
    split_logging_t data = {0};

    size_t size = rbuf_pop(ring_buffer, ARRAY_SIZE(data.buff), data.buff);

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

// master
void user_logging_master_poll(void) {
    split_logging_t data = {0};
    transaction_rpc_recv(RPC_ID_USER_LOGGING, sizeof(split_logging_t), &data);

    size_t size = data.header.bytes;
    if (size == 0) {
        return;
    }

    // copy received
    for (size_t i = 0; i < size; ++i) {
        rbuf_push(ring_buffer, data.buff[i]);
    }

    // flush if asked to
    if (data.header.flush || rbuf_full(ring_buffer)) {
        char   buff[PAYLOAD_SIZE];
        size_t size = rbuf_pop(ring_buffer, PAYLOAD_SIZE, buff);
        buff[size]  = '\0';

        // write it
        logging(LOG_DEBUG, "*****\n%s*****\n", buff);
    }
}
