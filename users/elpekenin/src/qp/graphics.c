// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/color.h>

#if defined(WPM_ENABLE)
#    include <quantum/wpm.h>
#endif

#include "elpekenin/build_info.h"
#include "elpekenin/crash.h"
#include "elpekenin/errno.h"
#include "elpekenin/layers.h"
#include "elpekenin/rng.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/map.h"
#include "elpekenin/utils/memory.h"
#include "elpekenin/utils/string.h"
#include "elpekenin/utils/time.h"

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

static deferred_executor_t    scrolling_text_executors[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS] = {0};
static scrolling_text_state_t scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS]    = {0};

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

// *** Asset handling ***

// Ported to zig :)

// *** Build info ***

void draw_commit(painter_device_t device) {
    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        _ = logging(QP, LOG_ERROR, "Font was NULL");
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

// *** Scrolling text ***

allocator_t *scrolling_allocator = &c_runtime_allocator;

static int render_scrolling_text_state(scrolling_text_state_t *state) {
    _ = logging(SCROLL, LOG_DEBUG, "%s: entry (char #%d)", __func__, (int)state->char_number);

    // prepare string slice
    char *slice = alloca(state->n_chars + 1); // +1 for null terminator
    if (slice == NULL) {
        _ = logging(SCROLL, LOG_ERROR, "%s: could not allocate", __func__);
        return -ENOMEM;
    }
    memset(slice, 0, state->n_chars + 1);

    uint8_t len = strlen(state->str);
    for (int8_t i = 0; i < state->n_chars; ++i) {
        uint8_t index = (state->char_number + i);

        // wrap to string length (plus spaces)
        uint8_t wrapped = index % (len + state->spaces);

        // copy a char or add separator whitespaces
        if (wrapped < len) {
            slice[i] = state->str[wrapped];
        } else {
            slice[i] = ' ';
        }
    }

    int16_t width = qp_textwidth(state->font, (const char *)slice);
    // clear previous rendering if needed
    if (state->width > 0) {
        qp_rect(state->device, state->x, state->y, state->x + state->width, state->y + state->font->line_height, HSV_BLACK, true);
    }
    state->width = width;

    // draw it
    bool ret = qp_drawtext_recolor(state->device, state->x, state->y, state->font, (const char *)slice, state->fg.hsv888.h, state->fg.hsv888.s, state->fg.hsv888.v, state->bg.hsv888.h, state->bg.hsv888.s, state->bg.hsv888.v);

    // update position
    if (ret) {
        ++state->char_number;
        if (state->char_number == len + state->spaces) {
            state->char_number = 0;
        }
    }

    log_success(ret, SCROLL, "%s", __func__);
    return ret;
}

static uint32_t scrolling_text_callback(uint32_t trigger_time, void *cb_arg) {
    scrolling_text_state_t *state = (scrolling_text_state_t *)cb_arg;
    int                     ret   = render_scrolling_text_state(state);

    // Setting the device to NULL clears the scrolling slot
    if (ret != 0) {
        state->device = NULL;
        return 0;
    }

    return state->delay;
}

deferred_token draw_scrolling_text(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay) {
    return draw_scrolling_text_recolor(device, x, y, font, str, n_chars, delay, 0, 0, 255, 0, 0, 0);
}

deferred_token draw_scrolling_text_recolor(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg) {
    _ = logging(SCROLL, LOG_DEBUG, "%s: entry", __func__);

    scrolling_text_state_t *scrolling_state = NULL;
    for (scrolling_text_state_t *state = scrolling_text_states; state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS]; ++state) {
        if (state->device == NULL) {
            scrolling_state = state;
            break;
        }
    }

    if (scrolling_state == NULL) {
        _ = logging(SCROLL, LOG_ERROR, "%s: fail (no free slot)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // make a copy of the string, to prevent issues if the original variable is removed
    // note: input is expected to end in null terminator
    uint8_t len          = strlen(str) + 1; // add one to also allocate/copy the terminator
    scrolling_state->str = malloc_with(scrolling_allocator, len);
    if (scrolling_state->str == NULL) {
        _ = logging(SCROLL, LOG_ERROR, "%s: fail (allocation)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // note, len is the buffer size we allocated right above (strlen(str) + 1)
    strlcpy(scrolling_state->str, str, len);

    // Prepare the scrolling state
    scrolling_state->device      = device;
    scrolling_state->x           = x;
    scrolling_state->y           = y;
    scrolling_state->font        = font;
    scrolling_state->n_chars     = n_chars;
    scrolling_state->delay       = delay;
    scrolling_state->char_number = 0;
    scrolling_state->spaces      = 5; // TODO: Receive as argument?
    scrolling_state->fg          = (qp_pixel_t){.hsv888 = {.h = hue_fg, .s = sat_fg, .v = val_fg}};
    scrolling_state->bg          = (qp_pixel_t){.hsv888 = {.h = hue_bg, .s = sat_bg, .v = val_bg}};

    // Draw the first string
    if (render_scrolling_text_state(scrolling_state) != 0) {
        _ = logging(SCROLL, LOG_ERROR, "%s: fail (render 1st step)", __func__);

        scrolling_state->device = NULL; // disregard the allocated scroling slot
        return INVALID_DEFERRED_TOKEN;
    }

    // Set up the timer
    scrolling_state->defer_token = defer_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, delay, scrolling_text_callback, scrolling_state);
    if (scrolling_state->defer_token == INVALID_DEFERRED_TOKEN) {
        _ = logging(SCROLL, LOG_ERROR, "%s: fail (setup executor)", __func__);

        scrolling_state->device = NULL; // disregard the allocated scrolling slot
        return INVALID_DEFERRED_TOKEN;
    }

    _ = logging(SCROLL, LOG_DEBUG, "%s: ok (deferred token = %d)", __func__, (int)scrolling_state->defer_token);
    return scrolling_state->defer_token;
}

void extend_scrolling_text(deferred_token scrolling_token, const char *str) {
    for (scrolling_text_state_t *state = scrolling_text_states; state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS]; ++state) {
        if (state->defer_token == scrolling_token) {
            uint8_t cur_len = strlen(state->str);
            uint8_t new_len = strlen(str);
            uint8_t len     = cur_len + new_len + 1;
            char   *new_pos = realloc_with(scrolling_allocator, state->str, len);

            if (new_pos == NULL) {
                _ = logging(SCROLL, LOG_ERROR, "%s: fail (realloc)", __func__);
                return;
            }
            state->str = new_pos;

            strlcat(state->str, str, len);

            return;
        }
    }
}

void stop_scrolling_text(deferred_token scrolling_token) {
    if (scrolling_token == INVALID_DEFERRED_TOKEN) {
        return;
    }

    for (scrolling_text_state_t *state = scrolling_text_states; state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS]; ++state) {
        if (state->defer_token == scrolling_token) {
            // Clear screen and de-allocate
            qp_rect(state->device, state->x, state->y, state->x + state->width, state->y + state->font->line_height, HSV_BLACK, true);

            free_with(scrolling_allocator, state->str);

            // Cleanup the state
            state->device      = NULL;
            state->defer_token = INVALID_DEFERRED_TOKEN;

            cancel_deferred_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, scrolling_token);

            return;
        }
    }

    _ = logging(QP, LOG_ERROR, "404 scrolling token");
}

// *** Callbacks ***

static uint32_t scrolling_text_tick_callback(uint32_t trigger_time, void *cb_arg) {
    static uint32_t last_scrolling_text_exec = 0;
    deferred_exec_advanced_task(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, &last_scrolling_text_exec);
    return 100; // 100ms sounds fast enough for me, text moving at +10 frames/second does not sound too readable for me
}

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

    size_t used = get_used_heap();

    char   buff[100];
    size_t buff_size = sizeof(buff);

    static size_t last_used = 0;

    if (args->device == NULL || args->font == NULL || last_used == used || state->running) {
        return 1000;
    }

    // on first run, draw flash size
    static bool flash = false;
    if (!flash) {
        flash = true;

        strlcpy(buff, "Flash: ", sizeof(buff));

        size_t offset = strlen(buff);
        pretty_bytes(get_used_flash(), buff + offset, buff_size - offset);

        strlcat(buff, "/", sizeof(buff));

        offset = strlen(buff);
        pretty_bytes(get_flash_size(), buff + offset, buff_size - offset);

        qp_drawtext(args->device, args->x, args->y - args->font->line_height, args->font, buff);
    }

    last_used      = used;
    state->running = true;

    size_t offset = 0;
    pretty_bytes(used, buff + offset, buff_size - offset);

    strlcat(buff, "/", sizeof(buff));

    offset = strlen(buff);
    pretty_bytes(get_heap_size(), buff + offset, buff_size - offset);

    // start the animation
    strlcpy(state->dest, buff, sizeof(state->dest));
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
    HSV color = {HSV_WHITE};

#    if defined(WPM_ENABLE)
    uint8_t wpm = get_current_wpm();

    if (wpm > 10) {
        color = (HSV){HSV_RED};
    }
    if (wpm > 30) {
        color = (HSV){HSV_YELLOW};
    }
    if (wpm > 50) {
        color = (HSV){HSV_GREEN};
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

static void elpekenin_qp_init(void) {
    load_qp_resources();

    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");
    if (font == NULL) {
        _ = logging(QP, LOG_ERROR, "Font was NULL");
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

    defer_exec(10, scrolling_text_tick_callback, NULL);
}
PEKE_PRE_INIT(elpekenin_qp_init, INIT_QP_TASKS_ARGS);
