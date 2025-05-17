// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/computer_stats.h"

#if defined(COMMUNITY_MODULE_QP_HELPERS_ENABLE)
#    include "qp_helpers.h"
#else
#    error Must enable 'drashna/qp_helpers'
#endif

static qp_callback_args_t args = {0};

static struct {
    uint8_t cpu[COMPUTER_STATS_ARRAY_SIZE];
    uint8_t ram[COMPUTER_STATS_ARRAY_SIZE];
} data = {0};

static bool redraw = false;

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_DEFINED(XAP_ENABLE) || !IS_DEFINED(COMMUNITY_MODULE_QP_HELPERS_ENABLE)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !redraw) {
        return MILLISECONDS(100);
    }

    const uint16_t max_value  = 100;
    const uint16_t graph_size = (uintptr_t)args->extra;

    const hsv_t axis_color = {HSV_WHITE};
    const hsv_t bg_color   = {HSV_BLACK};

    const graph_line_t graphs[] = {
        {
            .line_data  = data.cpu,
            .line_color = {HSV_BLUE},
            .mode       = LINE,
        },
        {
            .line_data  = data.ram,
            .line_color = {HSV_YELLOW},
            .mode       = LINE,
        },
    };

    qp_draw_graph(args->device, args->x, args->y, graph_size, graph_size, axis_color, bg_color, graphs, ARRAY_SIZE(graphs), COMPUTER_STATS_ARRAY_SIZE, max_value);
    redraw = false;

    return MILLISECONDS(200);
}

qp_callback_args_t *get_computer_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &args);

    return &args;
}

void push_computer_stats(uint8_t cpu, uint8_t ram) {
    memmove(data.cpu + 1, data.cpu, COMPUTER_STATS_ARRAY_SIZE - 1);
    memmove(data.ram + 1, data.ram, COMPUTER_STATS_ARRAY_SIZE - 1);

    data.cpu[0] = cpu;
    data.ram[0] = ram;

    redraw = true;
}
