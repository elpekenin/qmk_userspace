// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/keylog.h"

#include <quantum/quantum.h>
#include <string.h>
#include <tmk_core/protocol/host.h> // keyboard_led_state

#include "elpekenin/allocator.h" // memory_heap_t
#include "elpekenin/logging.h"
#include "elpekenin/map.h"
#include "elpekenin/sections.h"
#include "elpekenin/shortcuts.h"
#include "elpekenin/string.h"
#include "generated/keycode_str.h"

// *** Internal variables ***

#ifndef KEYLOG_SIZE
#    define KEYLOG_SIZE (40)
#endif

static bool keylog_dirty            = true;
static char keylog[KEYLOG_SIZE + 1] = {
    [0 ... KEYLOG_SIZE - 1] = ' ',
    [KEYLOG_SIZE]           = '\0',
}; // extra space for terminator

// *** Replacements implementation ***

typedef enum {
    NO_MODS,
    SHIFT,
    AL_GR,
    // ... implement more when needed
    __N_MODS__,
} active_mods_t;

typedef struct PACKED {
    const char *strings[__N_MODS__];
} replacements_t;

#define replacement(no_mods, shift, al_gr) \
    (replacements_t) {                     \
        .strings = {                       \
            [NO_MODS] = no_mods,           \
            [SHIFT]   = shift,             \
            [AL_GR]   = al_gr,             \
        },                                 \
    }

static new_map(replacements_t, replacements_map);

static replacements_t replacements_buff[100];
static memory_heap_t  replacements_heap;
static allocator_t    replacements_allocator;

static void replacements_init(void) {
    chHeapObjectInit(&replacements_heap, &replacements_buff, sizeof(replacements_buff));

    replacements_allocator = new_ch_heap_allocator(&replacements_heap, "replacements heap");

    map_init(replacements_map, 50, &replacements_allocator);

    // add replacements to the map
    // clang-format off
    map_set(replacements_map, "0",       replacement(NULL,  "=",  NULL));
    map_set(replacements_map, "1",       replacement(NULL,  "!",  "|" ));
    map_set(replacements_map, "2",       replacement(NULL,  "\"", "@" ));
    map_set(replacements_map, "3",       replacement(NULL,  NULL, "#" )); // · breaks keylog
    map_set(replacements_map, "4",       replacement(NULL,  "$",  "~" ));
    map_set(replacements_map, "5",       replacement(NULL,  "%",  NULL));
    map_set(replacements_map, "6",       replacement(NULL,  "&",  NULL));
    map_set(replacements_map, "7",       replacement(NULL,  "/",  NULL));
    map_set(replacements_map, "8",       replacement(NULL,  "(",  NULL));
    map_set(replacements_map, "9",       replacement(NULL,  ")",  NULL));
    map_set(replacements_map, "_______", replacement("__",  NULL, NULL));
    map_set(replacements_map, "AT",      replacement("@",   NULL, NULL));
    map_set(replacements_map, "BSLS",    replacement("\\",  NULL, NULL));
    map_set(replacements_map, "BSPC",    replacement("⇤",   NULL, NULL));
    map_set(replacements_map, "CAPS",    replacement("↕",   NULL, NULL));
    map_set(replacements_map, "COMM",    replacement(",",   ";",  NULL));
    map_set(replacements_map, "DB_TOGG", replacement("DBG", NULL, NULL));
    map_set(replacements_map, "DOT",     replacement(".",   ":",  NULL));
    map_set(replacements_map, "DOWN",    replacement("↓",   NULL, NULL));
    map_set(replacements_map, "ENT",     replacement("↲",   NULL, NULL));
    map_set(replacements_map, "GRV",     replacement("`",   "^",  NULL));
    map_set(replacements_map, "HASH",    replacement("#",   NULL, NULL));
    map_set(replacements_map, "LBRC",    replacement("[",   NULL, NULL));
    map_set(replacements_map, "LCBR",    replacement("{",   NULL, NULL));
    map_set(replacements_map, "LEFT",    replacement("←",   NULL, NULL));
    map_set(replacements_map, "LOWR",    replacement("▼",   NULL, NULL));
    map_set(replacements_map, "MINS",    replacement("-",   "_",  NULL));
    map_set(replacements_map, "NTIL",    replacement("´",   NULL, NULL));
    map_set(replacements_map, "R_SPC",   replacement(" ",   NULL, NULL));
    map_set(replacements_map, "RBRC",    replacement("]",   NULL, NULL));
    map_set(replacements_map, "RCBR",    replacement("}",   NULL, NULL));
    map_set(replacements_map, "RIGHT",   replacement("→",   NULL, NULL));
    map_set(replacements_map, "PLUS",    replacement("+",   "*",  NULL));
    map_set(replacements_map, "PIPE",    replacement("|",   NULL, NULL));
    map_set(replacements_map, "QUOT",    replacement("'",   "?",  NULL));
    map_set(replacements_map, "SPC",     replacement(" ",   NULL, NULL));
    map_set(replacements_map, "TAB",     replacement("⇥",   NULL, NULL));
    map_set(replacements_map, "TILD",    replacement("~",   NULL, NULL));
    map_set(replacements_map, "UP",      replacement("↑",   NULL, NULL));
    map_set(replacements_map, "UPPR",    replacement("▲",   NULL, NULL));
    map_set(replacements_map, "VOLU",    replacement("♪",   "♪",  NULL));
    // clang-format on
}
PEKE_PRE_INIT(replacements_init, INIT_KEYLOG_MAP);

// *** Formatting helpers ***

static void skip_prefix(const char **str) {
    char *prefixes[] = {"KC_", "RGB_", "QK_", "ES_", "TD_", "TL_"};

    for (uint8_t i = 0; i < ARRAY_SIZE(prefixes); ++i) {
        char   *prefix = prefixes[i];
        uint8_t len    = strlen(prefix);

        if (strncmp(prefix, *str, len) == 0) {
            *str += len;
            return;
        }
    }
}

static void maybe_symbol(const char **str) {
    int            ret;
    replacements_t replacements;

    // disable hash logging momentarily, as a lot of strings won't be in the replacements map
    WITHOUT_LOGGING(MAP, replacements = map_get(replacements_map, *str, ret););

    if (ret == -ENODEV) {
        return;
    }

    const char *target;
    switch (get_mods()) {
        case 0:
            target = replacements.strings[NO_MODS];
            break;

        case MOD_BIT_LCTRL:
        case MOD_BIT_RCTRL:
            target = replacements.strings[SHIFT];
            break;

        case MOD_BIT_RALT:
            target = replacements.strings[AL_GR];
            break;

        default:
            // nothing to be done here
            return;
    }

    // we may get here with a combination with no replacement, eg shift+arrows
    // dont want to assign str to NULL
    if (target != NULL) {
        *str = target;
    }
}

// convert to lowercase based on shift/caps
// overengineered so it can also work on strings and whatnot on future
static void apply_casing(const char **str) {
    // not a single char
    if (strlen(*str) > 1) {
        return;
    }

    // not a letter
    if (**str < 'A' || **str > 'Z') {
        return;
    }

    uint8_t mods  = get_mods();
    bool    shift = mods & MOD_MASK_SHIFT;
    bool    caps  = host_keyboard_led_state().caps_lock;

    // if writing uppercase, string already is, just return
    if (shift ^ caps) {
        return;
    }

    char *lowercase_letters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};

    *str = lowercase_letters[**str - 'A'];
}

// *** Updating the log ***

static void keylog_clear(void) {
    // spaces (not 0) so `qp_drawtext` actually renders something
    memset(keylog, ' ', KEYLOG_SIZE);
    keylog[KEYLOG_SIZE] = '\0';
}

static void _keylog_shift_right_byte(void) {
    for (uint8_t i = KEYLOG_SIZE - 1; i > 0; --i) {
        keylog[i] = keylog[i - 1];
    }
    keylog[0] = ' ';
}

static void keylog_shift_right(void) {
    // pop all utf-continuation bytes
    while (is_utf8_continuation(keylog[KEYLOG_SIZE - 1])) {
        _keylog_shift_right_byte();
    }

    // this is either an ascii char or the heading byte of utf
    _keylog_shift_right_byte();
}

static void keylog_shift_left(uint8_t len) {
    memmove(keylog, keylog + len, KEYLOG_SIZE - len);

    uint8_t counter = 0;
    while (is_utf8_continuation(keylog[0])) {
        memmove(keylog, keylog + 1, KEYLOG_SIZE - 1);
        ++counter;
    }

    // pad buffer to the right, to align after a utf8 symbol is deleted
    memmove(keylog + counter, keylog, KEYLOG_SIZE - counter);
    memset(keylog, ' ', counter);
}

static void keylog_append(const char *str) {
    uint8_t len = strlen(str);

    keylog_shift_left(len);
    for (uint8_t i = 0; i < len; ++i) {
        keylog[KEYLOG_SIZE - len + i] = str[i];
    }
}

// *** API ***

bool is_keylog_dirty(void) {
    return keylog_dirty;
}

const char *get_keylog(void) {
    // after code gets gets a "view", reset dirty-ness
    keylog_dirty = false;
    return keylog;
}

void keycode_repr(const char **str) {
    skip_prefix(str);
    maybe_symbol(str);
}

void keylog_process(uint16_t keycode, keyrecord_t *record) {
    // nothing on release (for now)
    if (!record->event.pressed) {
        return;
    }

    // dont want to show some keycodes
    // clang-format off
    if ((IS_QK_LAYER_TAP(keycode) && !record->tap.count)
        || keycode >= QK_USER  // dont want my custom keycodes on keylog
        || IS_RGB_KEYCODE(keycode)
        || IS_QK_LAYER_MOD(keycode)
        || IS_QK_MOMENTARY(keycode)
        || IS_QK_DEF_LAYER(keycode)
        || IS_MODIFIER_KEYCODE(keycode)
       )
    {
        // clang-format on
        return;
    }

    keylog_dirty = true;

    const char *str = get_keycode_name(keycode);

    uint8_t mods = get_mods();
    bool    ctrl = mods & MOD_MASK_CTRL;

    // delete from tail
    if (strstr(str, "BSPC") != NULL) {
        // ctrl + backspace clears whole log
        if (ctrl) {
            keylog_clear();
        }
        // backspace = remove last char
        else {
            keylog_shift_right();
        }
        return;
    }

    // unknown keycode, quit
    if (str == NULL) {
        return;
    }

    // convert string into symbols
    keycode_repr((const char **)&str);

    // casing is separate so that drawing keycodes on screen is always uppercase
    apply_casing((const char **)&str);

    keylog_append(str);
}
