// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/m5.h"

#include "elpekenin/xap.h"
#include "uart.h"

enum {
    MIC_START = N_XAP_MESSAGES,
    MIC_STOP,
} m5_id_t;

static void m5_send(const uint8_t *data, size_t data_len) {
    if (IS_ENABLED(M5_DEBUG)) {
        printf("[m5] sending: {");
        for (size_t i = 0; i < data_len; ++i) {
            printf(" %d", data[i]);
        }
        printf(" }\n");
    }

    uart_transmit(data, sizeof(data));
}

void m5_init(void) {
    uart_init(M5_BAUD_RATE);
}

#if IS_ENABLED(M5_MIC)
void m5_mic_start(void) {
    uart_write(MIC_START);
}

void m5_mic_stop(void) {
    uart_write(MIC_STOP);
}
#endif

#if IS_ENABLED(M5_MQTT)
static inline uint8_t lsb(uint16_t val) {
    return val & 0xFF;
}

static inline uint8_t msb(uint16_t val) {
    return (val >> 8) & 0xFF;
}

void m5_mqtt_screen_pressed(uint8_t screen_id, touch_report_t report) {
    // clang-format off
    uint8_t data[] = {
        SCREEN_PRESSED,
        screen_id,
        lsb(report.x), msb(report.x),
        lsb(report.y), msb(report.y),
    };
    // clang-format on

    m5_send(data, sizeof(data));
}

void m5_mqtt_screen_released(uint8_t screen_id) {
    uint8_t data[] = {SCREEN_RELEASED, screen_id};
    m5_send(data, sizeof(data));
}

void m5_mqtt_layer(layer_state_t layer_state) {
    uint8_t data[] = {LAYER_CHANGE, get_highest_layer(layer_state)};
    m5_send(data, sizeof(data));
}

void m5_mqtt_keyevent(uint16_t keycode, keyrecord_t *record) {
    // clang-format off
    uint8_t data[] = {
        KEYEVENT,
        lsb(keycode), msb(keycode),
        record->event.pressed,
    };
    // clang-format on

    m5_send(data, sizeof(data));
}
#endif
