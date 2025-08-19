// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/computer.h"

#include <quantum/compiler_support.h>

#include "elpekenin/xap.h"

STATIC_ASSERT(CM_ENABLED(QP_HELPERS), "Must enable 'drashna/qp_helpers'");
#include "qp_helpers.h"

typedef struct {
    size_t    points;
    ui_time_t last_update;
    uint8_t   cpu[COMPUTER_STATS_SIZE];
    uint8_t   ram[COMPUTER_STATS_SIZE];
} inner_state_t;

static inner_state_t state = {0};

// if types are ever changed, memmove needs update
STATIC_ASSERT(member_size(inner_state_t, cpu) == COMPUTER_STATS_SIZE, "memmove will use wrong size");
STATIC_ASSERT(member_size(inner_state_t, ram) == COMPUTER_STATS_SIZE, "memmove will use wrong size");

bool computer_init(ui_node_t *self) {
    return true;
}

ui_time_t computer_render(const ui_node_t *self, painter_device_t display) {
    computer_args_t *args = self->args;

    // clear after inactivity
    if (xap_last_activity_elapsed() > COMPUTER_STATS_UI_TIMEOUT) {
        if (args->clear) {
            args->clear = false;
            qp_rect(display, self->start.x, self->start.y, self->start.x + self->size.x, self->start.y + self->size.y, HSV_BLACK, true);
        }

        goto exit;
    }

    if (ui_time_lte(state.last_update, args->last_draw)) {
        goto exit;
    }

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
        .device = display,
        .start =
            {
                .x = self->start.x,
                .y = self->start.y,
            },
        .size =
            {
                // -1 just in case
                .x = self->size.x - 1,
                .y = self->size.y - 1,
            },
        .axis        = {HSV_WHITE},
        .background  = {HSV_BLACK},
        .data_points = state.points,
    };

    qp_draw_graph(&config, lines);

    args->last_draw = ui_time_now();
    args->clear     = true;

exit:
    return (ui_time_t)UI_MILLISECONDS(COMPUTER_STATS_UI_REDRAW_INTERVAL);
}

void push_computer(uint8_t cpu, uint8_t ram) {
    memmove(state.cpu + 1, state.cpu, COMPUTER_STATS_SIZE - 1);
    memmove(state.ram + 1, state.ram, COMPUTER_STATS_SIZE - 1);

    state.cpu[0] = cpu;
    state.ram[0] = ram;

    state.points = MIN(COMPUTER_STATS_SIZE, state.points + 1);

    state.last_update = ui_time_now();
}
