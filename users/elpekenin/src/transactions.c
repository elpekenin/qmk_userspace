// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/split/transactions.h"

#include <quantum/quantum.h>

#include "elpekenin/build_info.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/signatures.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"

// race condition, QMK fills the left/master fields **after** post_init funcs
// lets manually invoke it to make sure we get the right thing
bool is_keyboard_master_impl(void);

static inline void __split_size_err(void) {
    logging(SPLIT, LOG_ERROR, "%s size", __func__);
}

// *** Callbacks ***

void build_info_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != sizeof(build_info_t)) {
        return __split_size_err();
    }

    build_info_t* received_build_info = (build_info_t*)m2s_buffer;

    set_build_info(*received_build_info);
    build_info_sync_keymap_callback();
}

void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != sizeof(bool)) {
        return __split_size_err();
    }

    void shutdown_quantum(bool jump_to_bootloader);
    shutdown_quantum(*(bool*)m2s_buffer);
}

void user_ee_clr_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != 0) {
        return __split_size_err();
    }

    eeconfig_init();
}

extern void xap_receive_base(const void* data);

void user_xap_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != XAP_EPSIZE) {
        return __split_size_err();
    }

    xap_receive_base(m2s_buffer);
}

// *** Periodic tasks ***

static inline uint32_t build_info_sync_callback(uint32_t trigger_time, void* cb_arg) {
    build_info_t build_info = get_build_info();
    transaction_rpc_send(RPC_ID_BUILD_INFO, sizeof(build_info_t), &build_info);
    return 30000;
}

static inline uint32_t slave_log_sync_callback(uint32_t trigger_time, void* cb_arg) {
    user_logging_master_poll();
    return 3000;
}

// *** Exposed to other places ***

void reset_ee_slave(void) {
    if (!is_keyboard_master_impl()) {
        return;
    }

    transaction_rpc_send(RPC_ID_USER_EE_CLR, 0, NULL);
}

void xap_execute_slave(const void* data) {
    if (!is_keyboard_master_impl()) {
        return;
    }

    xap_split_msg_t msg = {0};
    // user, qp, operation = 3
    memcpy(&msg, data - sizeof(xap_request_header_t) - 3, XAP_EPSIZE);
    transaction_rpc_send(RPC_ID_XAP, XAP_EPSIZE, &msg);
}

// *** Register messages ***

static void split_init(void) {
    transaction_register_rpc(RPC_ID_BUILD_INFO, build_info_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
    transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
    transaction_register_rpc(RPC_ID_USER_EE_CLR, user_ee_clr_callback);
    transaction_register_rpc(RPC_ID_XAP, user_xap_callback);

    if (is_keyboard_master_impl()) {
        // 5 secs to prevent drawing on eInk right after flash
        // and issues if slave is not yet working (or w/eekenin))
        defer_exec(5000, build_info_sync_callback, NULL);
        defer_exec(5000, slave_log_sync_callback, NULL);
    }
}
PEKE_PRE_INIT(split_init, INIT_SPLIT);

// static void split_deinit(bool jump_to_bootloader) {
//     if (is_keyboard_master_impl()) {
//         transaction_rpc_send(RPC_ID_USER_SHUTDOWN, sizeof(jump_to_bootloader), &jump_to_bootloader);
//     }
// }
// PEKE_DEINIT(split_deinit, DEINIT_SPLIT);
