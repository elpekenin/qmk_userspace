// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/split/transactions.h"

#include <quantum/quantum.h>

#include "elpekenin/logging/backends/split.h"
#include "elpekenin/time.h"

STATIC_ASSERT(CM_ENABLED(LOGGING), "Must enable 'elpekenin/logging'");

#include "elpekenin/logging.h"

// compat: no work to do
#if !IS_ENABLED(SPLIT_LOG)
#    define SPLIT_LOG_SYNC_DELAY 0
#endif

// slave-side callbacks

static void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, __unused uint8_t s2m_size, __unused void* s2m_buffer) {
    if (m2s_size != sizeof(bool)) {
        logging(LOG_ERROR, "%s size", __func__);
        return;
    }

    void shutdown_quantum(bool jump_to_bootloader);
    shutdown_quantum(*(bool*)m2s_buffer);
}

static void user_ee_clr_callback(uint8_t m2s_size, __unused const void* m2s_buffer, __unused uint8_t s2m_size, __unused void* s2m_buffer) {
    if (m2s_size != 0) {
        logging(LOG_ERROR, "%s size", __func__);
        return;
    }

    eeconfig_init();
}

static void user_xap_callback(uint8_t m2s_size, const void* m2s_buffer, __unused uint8_t s2m_size, __unused void* s2m_buffer) {
    if (m2s_size != XAP_EPSIZE) {
        logging(LOG_ERROR, "%s size", __func__);
        return;
    }

    extern void xap_receive_base(const void* data);
    xap_receive_base(m2s_buffer);
}

static uint32_t slave_log_sync_task(__unused uint32_t trigger_time, __unused void* cb_arg) {
    if (!is_keyboard_master() || !IS_ENABLED(SPLIT_LOG)) {
        return 0;
    }

    return user_logging_master_poll();
}

void reset_ee_slave(void) {
    if (!is_keyboard_master()) {
        return;
    }

    transaction_rpc_send(RPC_ID_USER_EE_CLR, 0, NULL);
}

void xap_execute_slave(const void* data) {
    if (!is_keyboard_master() || !IS_ENABLED(XAP)) {
        return;
    }

    xap_split_msg_t msg = {0};
    memcpy(&msg.data, data, XAP_EPSIZE);
    transaction_rpc_send(RPC_ID_XAP, XAP_EPSIZE, &msg);
}

// register message handlers and kick off tasks
void transactions_init(void) {
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);

    if (IS_ENABLED(SPLIT_LOG)) {
        transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
    }

    transaction_register_rpc(RPC_ID_USER_EE_CLR, user_ee_clr_callback);

    if (IS_ENABLED(XAP)) {
        transaction_register_rpc(RPC_ID_XAP, user_xap_callback);
    }

    defer_exec(MILLISECONDS(SPLIT_LOG_SYNC_DELAY), slave_log_sync_task, NULL);
}
