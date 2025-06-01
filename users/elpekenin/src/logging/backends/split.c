// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ctype.h>
#include <quantum/compiler_support.h>
#include <quantum/quantum.h>
#include <quantum/split_common/transactions.h>
#include <quantum/util.h>

#if CM_ENABLED(TYPES)
#    include "elpekenin/types.h"
#else
#    error Must enable 'elpekenin/types'
#endif

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

OptionImpl(char);
RingBufferImpl(char);

static char rbuf_inner[SPLIT_LOG_BUFFER_SIZE] = {0};

// slave will write on its copy of this variable, master will copy (over split) onto its own
static RingBuffer(char) rbuf = rbuf_from(char, rbuf_inner);

int8_t sendchar_split(uint8_t chr) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return 0;
    }

    const bool pushed = rbuf.push(&rbuf, chr);
    if (!pushed) {
        // TODO: some warning or smth?
    }

    return 0;
}

void user_logging_slave_callback(__unused uint8_t m2s_size, __unused const void* m2s_buffer, __unused uint8_t s2m_size, void* s2m_buffer) {
    split_logging_t data = {0};

    for (size_t i = 0; i < PAYLOAD_SIZE - 1; ++i) {
        const Option(char) pop = rbuf.pop(&rbuf);
        if (!pop.is_some) {
            break;
        }

        const char chr = pop.unwrap(pop);
        if (chr == '\n') {
            data.header.flush = true;
        }

        data.buff[i]     = chr;
        data.buff[i + 1] = '\0';

        data.header.bytes += 1;
    }

    // copy data on send buffer
    memcpy(s2m_buffer, &data, sizeof(split_logging_t));
}

// master
void user_logging_master_poll(void) {
    split_logging_t data = {0};
    transaction_rpc_recv(RPC_ID_USER_LOGGING, sizeof(split_logging_t), &data);

    if (data.header.bytes == 0) {
        return;
    }

    // copy received
    bool rbuf_full = false;
    for (size_t i = 0; i < data.header.bytes; ++i) {
        const bool pushed = rbuf.push(&rbuf, data.buff[i]);
        if (!pushed) {
            rbuf_full = true;
            break;
        }
    }

    // flush if asked to
    if (data.header.flush || rbuf_full) {
        char buff[PAYLOAD_SIZE] = {0};

        for (size_t i = 0; i < PAYLOAD_SIZE - 1; ++i) {
            const Option(char) pop = rbuf.pop(&rbuf);
            if (!pop.is_some) {
                break;
            }

            buff[i]     = pop.unwrap(pop);
            buff[i + 1] = '\0';
        }

        // write it
        printf("split\n%s\nendsplit\n", buff);
    }
}
