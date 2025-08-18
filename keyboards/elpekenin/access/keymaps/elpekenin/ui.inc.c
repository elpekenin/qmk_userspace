// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/layers.h"
#include "generated/qp_resources.h" // access to fonts/images

#if CM_ENABLED(ALLOCATOR)
#    include "elpekenin/allocator.h"
#endif

#if CM_ENABLED(MEMORY)
#    include "elpekenin/memory.h"
#endif

#if IS_ENABLED(QP_LOG)
#    include "elpekenin/logging/backends/qp.h"
#endif

STATIC_ASSERT(CM_ENABLED(BUILD_ID), "Must enable 'elpekenin/build_id'");
#include "elpekenin/build_id.h"

// nodes provided in the module
#include "elpekenin/ui/layer.h"
#include "elpekenin/ui/rgb.h"
#include "elpekenin/ui/text.h"
#include "elpekenin/ui/uptime.h"
// custom nodes in userspace
#include "elpekenin/qp/ui/build_match.h"
#include "elpekenin/qp/ui/computer.h"
#include "elpekenin/qp/ui/github.h"

// clang really wants to indent things far to the right...

// clang-format off
static github_args_t gh_args = {
    .logo = gfx_github,
};

static build_id_args_t build_id_args = {
    .font     = font_fira_code,
    .interval = UI_MILLISECONDS(500),
};

static build_match_args_t build_match_args = {
    .font = font_fira_code,
};

static ui_node_t id[] = {
    {
        .node_size = UI_ABSOLUTE(15),
    },
    {
        .node_size = UI_FONT(1),
        .init      = build_id_init,
        .render    = build_id_render,
        .args      = &build_id_args,
    },
    {
        .node_size = UI_FONT(1),
        .init      = build_match_init,
        .render    = build_match_render,
        .args      = &build_match_args,
    },
};

static ui_node_t first_row[] = {
    {
        .node_size = UI_IMAGE(1),
        .init      = github_init,
        .render    = github_render,
        .args      = &gh_args,
    },
#if CM_ENABLED(BUILD_ID)
    {
        .node_size = UI_ABSOLUTE(5),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_TOP_BOTTOM,
        .children  = UI_CHILDREN(id),
    }
#endif
};

static uptime_args_t uptime_args = {
    .font = font_fira_code,
};

static layer_args_t layer_args = {
    .font       = font_fira_code,
    .interval   = UI_MILLISECONDS(100),
    .layer_name = get_layer_name,
};

#if CM_ENABLED(MEMORY)
static flash_args_t flash_args = {
    .font     = font_fira_code,
    .interval = UI_MILLISECONDS(500),
};
#endif

#if CM_ENABLED(ALLOCATOR)
static heap_args_t heap_args = {
    .font     = font_fira_code,
    .interval = UI_MILLISECONDS(500),
};
#endif

static computer_args_t computer_args = {};

static ui_node_t left[] = {
    {
        .node_size = UI_IMAGE(1),
        .direction = UI_SPLIT_DIR_LEFT_RIGHT,
        .children  = UI_CHILDREN(first_row),
        .args      = &gh_args,
    },
    {
        .node_size = UI_FONT(1),
        .init      = uptime_init,
        .render    = uptime_render,
        .args      = &uptime_args,
    },
    {
        .node_size = UI_FONT(1),
        .init      = layer_init,
        .render    = layer_render,
        .args      = &layer_args,
    },
#if CM_ENABLED(MEMORY)
    {
        .node_size = UI_FONT(1),
        .init      = flash_init,
        .render    = flash_render,
        .args      = &flash_args,
    },
#endif

#if CM_ENABLED(ALLOCATOR)
    {
        .node_size = UI_FONT(1),
        .init      = heap_init,
        .render    = heap_render,
        .args      = &heap_args,
    },
#endif

    {
        .node_size = UI_REMAINING(),
        .init      = computer_init,
        .render    = computer_render,
        .args      = &computer_args,
    }
};

static const uint8_t *const rgb_font = font_fira_code;
static const ui_time_t rgb_interval = UI_MILLISECONDS(500);

static rgb_args_t rgb_args = {
    .font     = rgb_font,
    .interval = rgb_interval,
};

static text_args_t rgb_mode_args = {
    .font     = rgb_font,
    .interval = rgb_interval,
    .str      = "Mode",
};

static text_args_t rgb_hsv_args = {
    .font     = rgb_font,
    .interval = rgb_interval,
    .str      = "HSV",
};

static ui_node_t rgb_mode[] = {
    {
        .node_size = UI_RELATIVE(20),
        .init      = text_init,
        .render    = text_render,
        .args      = &rgb_mode_args,
    },
    {
        .node_size = UI_REMAINING(),
        .init      = rgb_init,
        .render    = rgb_mode_render,
        .args      = &rgb_args,
    },
};

static ui_node_t rgb_hsv[] = {
    {
        .node_size = UI_RELATIVE(20),
        .init      = text_init,
        .render    = text_render,
        .args      = &rgb_hsv_args,
    },
    {
        .node_size = UI_REMAINING(),
        .init      = rgb_init,
        .render    = rgb_hsv_render,
        .args      = &rgb_args,
    },
};

static ui_node_t rgb[] = {
    {
        .node_size = UI_RELATIVE(50),
        .direction = UI_SPLIT_DIR_LEFT_RIGHT,
        .children  = UI_CHILDREN(rgb_mode),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_LEFT_RIGHT,
        .children  = UI_CHILDREN(rgb_hsv),
    },
};

#if CM_ENABLED(KEYLOG)
static keylog_args_t keylog_args = {
    .font = font_fira_code,
};
#endif

#if IS_ENABLED(QP_LOG)
static qp_logging_args_t qp_logging_args = {
    .font = font_fira_code,
};
#endif

static ui_node_t right[] = {
#if IS_ENABLED(RGB_MATRIX)
    {
        .node_size = UI_FONT(2),
        .direction = UI_SPLIT_DIR_TOP_BOTTOM,
        .children  = UI_CHILDREN(rgb),
        .args      = &rgb_args,
    },
#endif

#if CM_ENABLED(KEYLOG)
    {
        .node_size = UI_FONT(1),
        .init      = keylog_init,
        .render    = keylog_render,
        .args      = &keylog_args,
    },
#endif

#if IS_ENABLED(QP_LOG)
    {
        .node_size = UI_REMAINING(),
        .init      = qp_logging_init,
        .render    = qp_logging_render,
        .args      = &qp_logging_args,
    },
#endif
};

static ui_node_t nodes[] = {
    {
        .node_size = UI_RELATIVE(50),
        .direction = UI_SPLIT_DIR_TOP_BOTTOM,
        .children  = UI_CHILDREN(left),
    },
    {
        .node_size = UI_REMAINING(),
        .direction = UI_SPLIT_DIR_TOP_BOTTOM,
        .children  = UI_CHILDREN(right),
    },
};

ui_node_t root = {
    .direction = UI_SPLIT_DIR_LEFT_RIGHT,
    .children  = UI_CHILDREN(nodes),
};
// clang-format off
