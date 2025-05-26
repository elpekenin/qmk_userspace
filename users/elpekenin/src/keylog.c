// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/keylog.h"

#include <ctype.h>
#include <quantum/quantum.h>
#include <quantum/util.h>
#include <string.h>
#include <tmk_core/protocol/host.h> // keyboard_led_state

#include "elpekenin/string.h"

#if defined(COMMUNITY_MODULE_TYPES_ENABLE)
#    include "elpekenin/types.h"
#else
#    error Must enable 'elpekenin/types'
#endif

static bool keylog_dirty = true;

static char keylog[KCONF(KEYLOG_SIZE) + 1] = {
    [0 ... KCONF(KEYLOG_SIZE) - 1] = ' ',
    [KCONF(KEYLOG_SIZE)]           = '\0',
}; // extra space for terminator

typedef enum {
    NO_MODS,
    SHIFT,
    AL_GR,
    // ... implement more when needed
    N_MODS,
} active_mods_t;

typedef struct PACKED {
    const char *raw;
    const char *strings[N_MODS];
} replacements_t;

#define replacement(r, no_mods, shift, al_gr) \
    (replacements_t) {                        \
        .raw     = (r),                       \
        .strings = {                          \
            [NO_MODS] = (no_mods),            \
            [SHIFT]   = (shift),              \
            [AL_GR]   = (al_gr),              \
        },                                    \
    }

// clang-format off
static const replacements_t replacements[] = {
    replacement("0",       NULL,  "=",  NULL),
    replacement("1",       NULL,  "!",  "|" ),
    replacement("2",       NULL,  "\"", "@" ),
    replacement("3",       NULL,  NULL, "#" ), // · breaks keylog
    replacement("4",       NULL,  "$",  "~" ),
    replacement("5",       NULL,  "%",  NULL),
    replacement("6",       NULL,  "&",  NULL),
    replacement("7",       NULL,  "/",  NULL),
    replacement("8",       NULL,  "(",  NULL),
    replacement("9",       NULL,  ")",  NULL),
    replacement("_______", "__",  NULL, NULL),
    replacement("AT",      "@",   NULL, NULL),
    replacement("BSLS",    "\\",  NULL, NULL),
    replacement("BSPC",    "⇤",   NULL, NULL),
    replacement("CAPS",    "↕",   NULL, NULL),
    replacement("COMM",    ",",   ";",  NULL),
    replacement("DB_TOGG", "DBG", NULL, NULL),
    replacement("DOT",     ".",   ":",  NULL),
    replacement("DOWN",    "↓",   NULL, NULL),
    replacement("ENT",     "↲",   NULL, NULL),
    replacement("GRV",     "`",   "^",  NULL),
    replacement("HASH",    "#",   NULL, NULL),
    replacement("LBRC",    "[",   NULL, NULL),
    replacement("LCBR",    "{",   NULL, NULL),
    replacement("LEFT",    "←",   NULL, NULL),
    replacement("LOWR",    "▼",   NULL, NULL),
    replacement("MINS",    "-",   "_",  NULL),
    replacement("NTIL",    "´",   NULL, NULL),
    replacement("R_SPC",   " ",   NULL, NULL),
    replacement("RBRC",    "]",   NULL, NULL),
    replacement("RCBR",    "}",   NULL, NULL),
    replacement("RIGHT",   "→",   NULL, NULL),
    replacement("PLUS",    "+",   "*",  NULL),
    replacement("PIPE",    "|",   NULL, NULL),
    replacement("QUOT",    "'",   "?",  NULL),
    replacement("SPC",     " ",   NULL, NULL),
    replacement("TAB",     "⇥",   NULL, NULL),
    replacement("TILD",    "~",   NULL, NULL),
    replacement("UP",      "↑",   NULL, NULL),
    replacement("UPPR",    "▲",   NULL, NULL),
    replacement("VOLU",    "♪",   "♪",  NULL),
};
// clang-format on

static void skip_prefix(const char **str) {
    char *prefixes[] = {"KC_", "RGB_", "QK_", "ES_", "TD_", "TL_"};

    for (size_t i = 0; i < ARRAY_SIZE(prefixes); ++i) {
        char   *prefix = prefixes[i];
        uint8_t len    = strlen(prefix);

        if (strncmp(prefix, *str, len) == 0) {
            *str += len;
            return;
        }
    }
}

OptionImpl(replacements_t);

static Option(replacements_t) find_replacement(const char *str) {
    for (size_t i = 0; i < ARRAY_SIZE(replacements); ++i) {
        const replacements_t replacement = replacements[i];

        if (strcmp(replacement.raw, str) == 0) {
            return Some(replacements_t, replacement);
        }
    }

    return None(replacements_t);
}

static void maybe_symbol(const char **str) {
    const Option(replacements_t) maybe_replacement = find_replacement(*str);
    if (!maybe_replacement.is_some) {
        return;
    }

    replacements_t replacement = unwrap(maybe_replacement);

    const char *target = NULL;
    switch (get_mods()) {
        case 0:
            target = replacement.strings[NO_MODS];
            break;

        case MOD_BIT_LSHIFT:
        case MOD_BIT_RSHIFT:
            target = replacement.strings[SHIFT];
            break;

        case MOD_BIT_RALT:
            target = replacement.strings[AL_GR];
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
    if (!isalpha((unsigned char)**str)) {
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

static void keylog_clear(void) {
    // spaces (not 0) so `qp_drawtext` actually renders something
    memset(keylog, ' ', KCONF(KEYLOG_SIZE));
    keylog[KCONF(KEYLOG_SIZE)] = '\0';
}

static void keylog_shift_right_one_byte(void) {
    memmove(keylog + 1, keylog, KCONF(KEYLOG_SIZE) - 1);
    keylog[0] = ' ';
}

static void keylog_shift_right(void) {
    // pop all utf-continuation bytes
    while (is_utf8_continuation(keylog[KCONF(KEYLOG_SIZE) - 1])) {
        keylog_shift_right_one_byte();
    }

    // this is either an ascii char or the heading byte of utf
    keylog_shift_right_one_byte();
}

static void keylog_shift_left(uint8_t len) {
    memmove(keylog, keylog + len, KCONF(KEYLOG_SIZE) - len);

    uint8_t counter = 0;
    while (is_utf8_continuation(keylog[0])) {
        memmove(keylog, keylog + 1, KCONF(KEYLOG_SIZE) - 1);
        ++counter;
    }

    // pad buffer to the right, to align after a utf8 symbol is deleted
    memmove(keylog + counter, keylog, KCONF(KEYLOG_SIZE) - counter);
    memset(keylog, ' ', counter);
}

static void keylog_append(const char *str) {
    uint8_t len = strlen(str);

    keylog_shift_left(len);
    for (uint8_t i = 0; i < len; ++i) {
        keylog[KCONF(KEYLOG_SIZE) - len + i] = str[i];
    }
}

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

    const char *str = get_keycode_string(keycode);

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
    keycode_repr(&str);

    // casing is separate so that drawing keycodes on screen is always uppercase
    apply_casing(&str);

    keylog_append(str);
}
