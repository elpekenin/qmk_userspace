// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/ui/computer.h"

#include <quantum/compiler_support.h>

#include "elpekenin/xap.h"

STATIC_ASSERT(CM_ENABLED(QP_HELPERS), "Must enable 'drashna/qp_helpers'");
#include "qp_helpers.h"

typedef struct {
    size_t   points;
    uint32_t last;
    uint8_t  cpu[QP_TASK_COMPUTER_SIZE];
    uint8_t  ram[QP_TASK_COMPUTER_SIZE];
} inner_state_t;

static inner_state_t state = {0};

// if types are ever changed, memmove needs update
STATIC_ASSERT(member_size(inner_state_t, cpu) == QP_TASK_COMPUTER_SIZE, "memmove will use wrong size");
STATIC_ASSERT(member_size(inner_state_t, ram) == QP_TASK_COMPUTER_SIZE, "memmove will use wrong size");

bool computer_init(ui_node_t *self) {
    return true;
}

void computer_render(const ui_node_t *self, painter_device_t display) {
    computer_args_t *args = self->args;

    if (!task_should_draw(&args->timer, MILLISECONDS(QP_TASK_COMPUTER_REDRAW_INTERVAL))) {
        return;
    }

    // clear after inactivity
    if (xap_last_activity_elapsed() > MILLISECONDS(QP_TASK_COMPUTER_TIMEOUT)) {
        if (args->clear) {
            args->clear = false;
            qp_rect(display, self->start.x, self->start.y, self->start.x + self->size.x, self->start.y + self->size.y, HSV_BLACK, true);
        }

        return;
    }

    if (state.last <= args->last) {
        return;
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

    args->last  = timer_read32();
    args->clear = true;
}

void push_computer(uint8_t cpu, uint8_t ram) {
    memmove(state.cpu + 1, state.cpu, QP_TASK_COMPUTER_SIZE - 1);
    memmove(state.ram + 1, state.ram, QP_TASK_COMPUTER_SIZE - 1);

    state.cpu[0] = cpu;
    state.ram[0] = ram;

    state.points = MIN(QP_TASK_COMPUTER_SIZE, state.points + 1);

    state.last = timer_read32();
}
