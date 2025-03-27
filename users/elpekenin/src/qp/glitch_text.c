// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/qp/glitch_text.h"

#include "elpekenin/qp/graphics.h"

#if defined(COMMUNITY_MODULE_RNG_ENABLE)
#    include "elpekenin/rng.h"
#endif

static uint16_t gen_random_pos(uint16_t max, uint64_t *mask) {
    uint16_t pos = 0;

    do { // dont mess already-done char
#if defined(COMMUNITY_MODULE_RNG_ENABLE)
        pos = rng_min_max(0, max);
#else
        pos += 1;
#endif
    } while ((*mask & (1 << pos)));

    *mask |= (1 << pos);
    return pos;
}

uint32_t glitch_text_callback(uint32_t trigger_time, void *cb_arg) {
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
#if defined(COMMUNITY_MODULE_RNG_ENABLE)
        chr = rng_min_max('!', '~');
#else
        chr = '!';
#endif
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
