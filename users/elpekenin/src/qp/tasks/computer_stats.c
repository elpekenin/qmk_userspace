// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/tasks/computer_stats.h"

#include <quantum/compiler_support.h>

#include "elpekenin/xap.h"

STATIC_ASSERT(CM_ENABLED(QP_HELPERS), "Must enable 'drashna/qp_helpers'");

#include "qp_helpers.h"

static struct {
    qp_callback_args_t args;
    bool               redraw;
    bool               clear;
    size_t             points;
    uint8_t            cpu[QP_TASK_COMPUTER_STATS_SIZE];
    uint8_t            ram[QP_TASK_COMPUTER_STATS_SIZE];
} computer_stats = {0};
STATIC_ASSERT(sizeof(*computer_stats.cpu) == 1, "memmove will use wrong size");
STATIC_ASSERT(sizeof(*computer_stats.ram) == 1, "memmove will use wrong size");

static uint32_t callback(__unused uint32_t trigger_time, void *cb_arg) {
    if (!IS_ENABLED(XAP) || !CM_ENABLED(QP_HELPERS)) {
        return 0;
    }

    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return MILLISECONDS(QP_TASK_COMPUTER_STATS_REDRAW_INTERVAL);
    }

    const uint16_t graph_size = (uintptr_t)args->extra;

    if (xap_last_activity_elapsed() > MILLISECONDS(QP_TASK_COMPUTER_STATS_TIMEOUT)) {
        if (computer_stats.clear) {
            qp_rect(args->device, args->x, args->y, args->x + graph_size, args->y + graph_size, HSV_BLACK, true);
        }

        computer_stats.clear = false;
    } else {
        computer_stats.clear = true;

        const graph_line_t lines[] = {
            {
                .data      = computer_stats.cpu,
                .color     = {HSV_BLUE},
                .mode      = LINE,
                .max_value = 100,
            },
            {
                .data      = computer_stats.ram,
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
            .data_points = computer_stats.points,
        };

        if (computer_stats.redraw) {
            qp_draw_graph(&config, lines);
        }

        computer_stats.redraw = false;
    }

    return MILLISECONDS(QP_TASK_COMPUTER_STATS_REDRAW_INTERVAL);
}

qp_callback_args_t *get_computer_stats_args(void) {
    static bool configured = false;
    if (configured) {
        return NULL;
    }
    configured = true;

    defer_exec(MILLISECONDS(10), callback, &computer_stats.args);

    return &computer_stats.args;
}

void push_computer_stats(uint8_t cpu, uint8_t ram) {
    memmove(computer_stats.cpu + 1, computer_stats.cpu, QP_TASK_COMPUTER_STATS_SIZE - 1);
    memmove(computer_stats.ram + 1, computer_stats.ram, QP_TASK_COMPUTER_STATS_SIZE - 1);

    computer_stats.cpu[0] = cpu;
    computer_stats.ram[0] = ram;

    computer_stats.points = MIN(QP_TASK_COMPUTER_STATS_SIZE, computer_stats.points + 1);
    computer_stats.redraw = true;
}
