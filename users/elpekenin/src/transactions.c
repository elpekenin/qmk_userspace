// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/split/transactions.h"

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/split.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");
#include "elpekenin/logging.h"

// compat: no work to do
#if !IS_ENABLED(SPLIT_LOG)
#    define SPLIT_LOG_SYNC_DELAY 0
#endif

typedef struct {
    bool ok;
    u128 value;
} build_id_msg_t;

//
// Slave-side handlers
//

static void ee_clear_handler(uint8_t m2s_size, __unused const void* m2s_buffer, __unused uint8_t s2m_size, __unused void* s2m_buffer) {
    if (m2s_size != 0) {
        logging(LOG_ERROR, "%s size", __func__);
        return;
    }

    eeconfig_init();
}

static void xap_handler(uint8_t m2s_size, const void* m2s_buffer, __unused uint8_t s2m_size, __unused void* s2m_buffer) {
    if (m2s_size != XAP_EPSIZE) {
        logging(LOG_ERROR, "%s size", __func__);
        return;
    }

    extern void xap_receive_base(const void* data);
    xap_receive_base(m2s_buffer);
}

static void build_id_handler(__unused uint8_t m2s_size, __unused const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (s2m_size != sizeof(build_id_msg_t)) {
        return;
    }

    build_id_msg_t* msg = s2m_buffer;

    u128       build_id;
    const bool fail = get_build_id(&build_id) < 0;

    msg->ok = !fail;
    if (!fail) {
        msg->value = build_id;
    }
}

//
// Periodic tasks callbacks
//

static uint32_t logging_task_cb(__unused uint32_t trigger_time, __unused void* cb_arg) {
    if (!is_keyboard_master() || !IS_ENABLED(SPLIT_LOG)) {
        return 0;
    }

    return user_logging_master_poll();
}

//
// Public API
//

void reset_ee_slave(void) {
    if (!is_keyboard_master()) {
        return;
    }

    transaction_rpc_send(RPC_ID_USER_EEPROM_CLEAR, 0, NULL);
}

void xap_execute_slave(const void* data) {
    if (!is_keyboard_master() || !IS_ENABLED(XAP)) {
        return;
    }

    xap_split_msg_t msg = {0};
    memcpy(&msg.data, data, XAP_EPSIZE);
    transaction_rpc_send(RPC_ID_USER_XAP, XAP_EPSIZE, &msg);
}

bool get_slave_build_id(u128* build_id) {
    if (!is_keyboard_master()) {
        return false;
    }

    build_id_msg_t msg = {0};

    const bool ret = transaction_rpc_recv(RPC_ID_USER_BUILD_ID, sizeof(build_id_msg_t), &msg);
    if (!ret || !msg.ok) {
        return false;
    }

    *build_id = msg.value;
    return true;
}

void transactions_init(void) {
    //
    // rpc handlers
    //
    if (IS_ENABLED(SPLIT_LOG)) {
        transaction_register_rpc(RPC_ID_USER_LOGGING, logging_handler);
    }

    transaction_register_rpc(RPC_ID_USER_EEPROM_CLEAR, ee_clear_handler);

    if (IS_ENABLED(XAP)) {
        transaction_register_rpc(RPC_ID_USER_XAP, xap_handler);
    }

    transaction_register_rpc(RPC_ID_USER_BUILD_ID, build_id_handler);

    //
    // periodic tasks
    //
    defer_exec(SPLIT_LOG_SYNC_DELAY, logging_task_cb, NULL);
}
