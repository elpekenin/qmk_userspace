// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/graphics.h"

#include <errno.h>
#include <quantum/quantum.h>

#include "elpekenin/allocator.h"
#include "elpekenin/build_info.h"
#include "elpekenin/layers.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/memory.h"
#include "elpekenin/qp/assets.h"
#include "elpekenin/rng.h"
#include "elpekenin/string.h"
#include "elpekenin/time.h"

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif

// *** Macro magic for defer_exec handling ***

#define TASK(__name)                                                                                  \
    static deferred_token     __name##_token = INVALID_DEFERRED_TOKEN;                                \
    static qp_callback_args_t __name##_args  = {0};                                                   \
    static uint32_t           __name##_task_callback(uint32_t trigger_time, void *cb_arg);            \
    void                      set_##__name##_device(painter_device_t device) {                        \
        __name##_args.device = device;                                           \
        cancel_deferred_exec(__name##_token);                                    \
        __name##_token = defer_exec(10, __name##_task_callback, &__name##_args); \
    }

// *** Internal variables ***

TASK(logging)
TASK(uptime)
TASK(heap_stats)
TASK(layer)
#if defined(KEYLOG_ENABLE)
TASK(keylog)
#endif

typedef enum {
    FILLING,
    COPYING,
    DONE,
} anim_phase_t;

typedef struct {
    size_t       used;
    bool         running;
    uint64_t     mask;
    anim_phase_t state;
    char         curr[65]; // u64 mask + '\0'
    char         dest[65]; // u64 mask + '\0'
    void (*callback)(void *);
} glitch_text_state_t;

static glitch_text_state_t heap_stats_extra = {0};
static glitch_text_state_t layer_extra      = {0};

// *** Build info ***

void draw_commit(painter_device_t device) {
    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        logging(QP, LOG_ERROR, "Font was NULL");
        return;
    }

    painter_driver_t *driver     = (painter_driver_t *)device;
    uint16_t          width      = driver->panel_width;
    uint16_t          height     = driver->panel_height;
    int16_t           hash_width = qp_textwidth(font, get_build_info().commit);

    uint16_t real_width  = width;
    uint16_t real_height = height;

    if (driver->rotation == QP_ROTATION_90 || driver->rotation == QP_ROTATION_270) {
        real_width  = height;
        real_height = width;
    }

    uint16_t x = real_width - hash_width;
    uint16_t y = real_height - font->line_height;

    qp_drawtext_recolor(device, x, y, font, get_build_info().commit, HSV_RED, HSV_WHITE);
}

// *** Callbacks ***

static uint32_t logging_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return 100;
}

static uint32_t uptime_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL) {
        return 1000;
    }

    div_t result = div(trigger_time, MS_IN_A_DAY);
    // uint8_t days = result.quot;

    result          = div(result.rem, MS_IN_AN_HOUR);
    uint8_t hours   = result.quot;
    result          = div(result.rem, MS_IN_A_MIN);
    uint8_t minutes = result.quot;
    uint8_t seconds = result.rem / MS_IN_A_SEC;

    char uptime_str[16];
    snprintf(uptime_str, sizeof(uptime_str), "Up|%02dh%02dm%02ds", hours, minutes, seconds);

    qp_drawtext(args->device, args->x, args->y, args->font, uptime_str);

    return 1000;
}

static uint16_t gen_random_pos(uint16_t max, uint64_t *mask) {
    uint16_t pos;

    do { // dont mess already-done char
        pos = rng_min_max(0, max);
    } while ((*mask & (1 << pos)));

    *mask |= (1 << pos);
    return pos;
}

static void draw_heap(void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // heading space to align the ":" with flash
    int16_t width = qp_drawtext(args->device, args->x, args->y, args->font, " Heap: ");

    qp_drawtext(args->device, args->x + width, args->y, args->font, state->curr);
}

static uint32_t glitch_text_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    // getting here means arguments are none, executing this function is guarded by checks

    // strings converged, draw and quit
    if (state->state == DONE) {
        state->running = false;

        strlcpy(state->curr, state->dest, sizeof(state->curr));
        // keep terminator untouched
        memset(state->dest, ' ', ARRAY_SIZE(state->dest) - 1);

        state->callback(args);
        return 0;
    }

    // actual logic
    size_t len = strlen(state->dest);

    char chr = '\0';
    do { // dont want a terminator mid-string
        chr = rng_min_max('!', '~');
    } while (chr == '\0');

    // all bits that should be set are set, change state
    uint64_t mask = (1 << (len - 1)) - 1;
    if ((state->mask & mask) == mask) {
        state->mask = 0;
        switch (state->state) {
            case FILLING:
                state->state = COPYING;
                break;

            case COPYING:
                state->state = DONE;
                break;

            case DONE:
                break;
        }
    }

    // this is an index, -1 prevents out of bounds str[len]
    uint16_t pos = gen_random_pos(len - 1, &state->mask);

    switch (state->state) {
        case FILLING:
            state->curr[pos] = chr;
            break;

        case COPYING:
            state->curr[pos] = state->dest[pos];
            break;

        case DONE:
            break;
    }

    state->callback(args);
    return 30;
}

static uint32_t heap_stats_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    size_t used_heap = get_used_heap();

    string_t str = new_string(100);
    int      len = 0;

    static size_t last_used = 0;

    if (args->device == NULL || args->font == NULL || last_used == used_heap || state->running) {
        return 1000;
    }

    // on first run, draw flash size
    static bool flash = false;
    if (!flash) {
        flash = true;

        len = lcpy(&str, "Flash: ");
        len += pretty_bytes(&str, get_used_flash());
        len += lcat(&str, "/");
        len += pretty_bytes(&str, get_flash_size());

        qp_drawtext(args->device, args->x, args->y - args->font->line_height, args->font, str.ptr - len);

        // reset buffer
        str.ptr -= len;
        str.len += len;
    }

    last_used      = used_heap;
    state->running = true;

    len = pretty_bytes(&str, used_heap);
    len += lcat(&str, "/");
    len += pretty_bytes(&str, get_heap_size());

    // start the animation
    strlcpy(state->dest, str.ptr - len, sizeof(state->dest));
    state->mask  = 0;
    state->state = FILLING;
    defer_exec(10, glitch_text_callback, args);

    return 1000;
}

#if defined(KEYLOG_ENABLE)
static uint32_t keylog_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    if (args->device == NULL || args->font == NULL || !is_keylog_dirty()) {
        return 10;
    }

    const char *keylog = get_keylog();
    uint16_t    width  = qp_get_width(args->device);
    uint16_t    height = qp_get_height(args->device);

    int16_t textwidth = qp_textwidth(args->font, keylog);
    qp_rect(args->device, 0, height - args->font->line_height, width - textwidth, height, HSV_BLACK, true);

    // default to white, change it based on WPM (if enabled)
    hsv_t color = {HSV_WHITE};

#    if defined(WPM_ENABLE)
    uint8_t wpm = get_current_wpm();

    if (wpm > 10) {
        color = (hsv_t){HSV_RED};
    }
    if (wpm > 30) {
        color = (hsv_t){HSV_YELLOW};
    }
    if (wpm > 50) {
        color = (hsv_t){HSV_GREEN};
    }
#    endif

    qp_drawtext_recolor(args->device, width - textwidth, height - args->font->line_height, args->font, keylog, color.h, color.s, color.v, HSV_BLACK);

    return 10;
}
#endif

static void draw_layer(void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    if (args->device == NULL || args->font == NULL) {
        return;
    }

    // random color
    // sat = 0 => white regardless of hue
    uint8_t hue = rng_min_max(0, 255);
    uint8_t sat = state->running ? 255 : 0;

    qp_drawtext_recolor(args->device, args->x, args->y, args->font, state->curr, hue, sat, 255, HSV_BLACK);
}

static uint32_t layer_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t  *args  = (qp_callback_args_t *)cb_arg;
    glitch_text_state_t *state = (glitch_text_state_t *)args->extra;

    static uint8_t last_layer = UINT8_MAX;

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    if (args->device == NULL || args->font == NULL || last_layer == layer || state->running) {
        return 100;
    }

    last_layer     = layer;
    state->running = true;

    // start the animation
    strlcpy(state->dest, get_layer_name(layer), sizeof(state->dest));
    state->mask  = 0;
    state->state = FILLING;
    defer_exec(10, glitch_text_callback, args);

    return 100;
}

void qp_tasks_init(void) {
    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        logging(QP, LOG_ERROR, "Font was NULL");
        return;
    }

    // positions are hard-coded for ILI9341 on access

    size_t spacing = font->line_height + 2;

    logging_args.font                   = font;
    logging_args.x                      = 160;
    logging_args.y                      = 100;
    logging_args.scrolling_args.n_chars = 18;
    logging_args.scrolling_args.delay   = 500;

    uptime_args.font = font;
    uptime_args.x    = 50;
    uptime_args.y    = 55;

    heap_stats_args.font      = font;
    heap_stats_args.x         = 50;
    heap_stats_args.y         = uptime_args.y + 2 * spacing;
    heap_stats_extra.callback = draw_heap;
    heap_stats_args.extra     = &heap_stats_extra;

    layer_args.font      = font;
    layer_args.x         = 70;
    layer_args.y         = heap_stats_args.y + spacing;
    layer_extra.callback = draw_layer;
    layer_args.extra     = &layer_extra;

#if defined(KEYLOG_ENABLE)
    keylog_args.font = fira_code;
#endif
}
