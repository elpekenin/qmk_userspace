// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/computer_stats.h"

#include "elpekenin/xap.h"

#if defined(COMMUNITY_MODULE_QP_HELPERS_ENABLE)
#    include "qp_helpers.h"
#else
#    error Must enable 'drashna/qp_helpers'
#endif

static struct {
    qp_callback_args_t args;
    bool               redraw;
    bool               clear;
    uint8_t            points;
    uint8_t            cpu[KCONF(QP_COMPUTER_STATS_SIZE)];
    uint8_t            ram[KCONF(QP_COMPUTER_STATS_SIZE)];
} state = {0};

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(XAP) || !IS_ENABLED(COMMUNITY_MODULE_QP_HELPERS)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return MILLISECONDS(100);
    }

    const uint16_t graph_size = (uintptr_t)args->extra;

    if (xap_last_activity_elapsed() > SECONDS(5)) {
        if (state.clear) {
            qp_rect(args->device, args->x, args->y, args->x + graph_size, args->y + graph_size, HSV_BLACK, true);
        }

        state.clear = false;
    } else {
        state.clear = true;

        const graph_line_t lines[] = {
            {
                .data      = state.cpu,
                .color     = {HSV_BLUE},
                .mode      = LINE,
                .max_value = 100,
            },
            {
                .data      = state.ram,
                .color     = {HSV_YELLOW},
                .mode      = LINE,
                .max_value = 100,
            },
            GRAPHS_END,
        };

        const graph_config_t config = {
            .device = args->device,
            .start =
                {
                    .x = args->x,
                    .y = args->y,
                },
            .size =
                {
                    .x = graph_size,
                    .y = graph_size,
                },
            .axis        = {HSV_WHITE},
            .background  = {HSV_BLACK},
            .data_points = state.points,
        };

        if (state.redraw) {
            qp_draw_graph(&config, lines);
        }

        state.redraw = false;
    }

    return MILLISECONDS(200);
}

qp_callback_args_t *get_computer_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &state.args);

    return &state.args;
}

void push_computer_stats(uint8_t cpu, uint8_t ram) {
    memmove(state.cpu + 1, state.cpu, KCONF(QP_COMPUTER_STATS_SIZE) - 1);
    memmove(state.ram + 1, state.ram, KCONF(QP_COMPUTER_STATS_SIZE) - 1);

    state.cpu[0] = cpu;
    state.ram[0] = ram;

    state.points = MIN(KCONF(QP_COMPUTER_STATS_SIZE), state.points + 1);
    state.redraw = true;
}
