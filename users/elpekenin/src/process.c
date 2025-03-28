// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/keymap_extras/sendstring_spanish.h>
#include <quantum/process_keycode/process_rgb_matrix.h>
#include <quantum/quantum.h>

#include "elpekenin/keycodes.h"
#include "elpekenin/logging.h"
#include "elpekenin/memory.h"
#include "elpekenin/shortcuts.h"
#include "elpekenin/signatures.h"
#include "elpekenin/string.h"
#include "generated/keycode_str.h"

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/logging/backends/qp.h"
#endif

#if defined(SPLIT_KEYBOARD)
#    include "elpekenin/split/transactions.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

// *** Logic start ***

#if defined(AUTOCORRECT_ENABLE)
// dont mind me, just bodging my way in  :)
static bool last_td_spc = false;

bool apply_autocorrect(uint8_t backspaces, const char *str, char *typo, char *correct) {
    logging(UNKNOWN, LOG_WARN, "'%s' - '%s'", typo, correct);

    // regular handle
    if (!last_td_spc) {
        return true;
    }

    // on space tap dance
    // ... fix the typo
    for (uint8_t i = 0; i < backspaces; ++i) {
        tap_code(BSPC);
    }
    send_string_P(str);

    // ... and add the actual space
    tap_code(SPC);

    return false;
}

bool process_autocorrect_user(uint16_t *keycode, keyrecord_t *record, uint8_t *typo_buffer_size, uint8_t *mods) {
    last_td_spc = false;

    switch (*keycode) {
        case TD_SPC:
            *keycode              = KC_SPC; // make this look like a regular spacebar
            record->event.pressed = false;  // trigger an extra backspace when corrected
            last_td_spc           = true;
            break;

        case TD_Z:
            *keycode = KC_Z;
            break;

        default:
            break;
    }
    return process_autocorrect_default_handler(keycode, record, typo_buffer_size, mods);
}
#endif

#if defined(KEYLOG_ENABLE)
bool keylog_enabled = true;
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    string_t str = new_string(15);

#if defined(KEYLOG_ENABLE)
    if (keylog_enabled) {
        keylog_process(keycode, record);
    }
#endif

#if defined(XAP_ENABLE)
    // log events over XAP
    xap_keyevent(keycode, record);
#endif

#if defined(RGB_MATRIX_ENABLE)
    if (IS_RGB_MATRIX_KEYCODE(keycode)) {
        if (record->event.pressed) {
            logging(UNKNOWN, LOG_INFO, "Used %s", get_keycode_name(keycode));
        }

        bool ret;
        WITHOUT_DEBUG(ret = process_rgb_matrix(keycode, record););
        return ret;
    }
#endif

    if (!process_record_keymap(keycode, record)) {
        return false;
    }

    bool    pressed = record->event.pressed;
    uint8_t mods    = get_mods();
    bool    l_sft   = mods & MOD_BIT(KC_LSFT);

    switch (keycode) {
#if defined(SPLIT_KEYBOARD)
        case EE_CLR:
            if (pressed) {
                reset_ee_slave(); // reset on slave too
            }
            return true;
#endif

        case PK_CPYR:
            // avoid messing up when i press GUI instead of TRI_LAYER for QK_RST
            if (get_mods() & MOD_MASK_GUI) {
                return false;
            }

            if (pressed) {
                // clang-format off
                send_string(
                    "// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>\n"
                    "// SPDX-License-Identifier: GPL-2.0-or-later\n"
                );
                // clang-format on
            }
            return false;

#if defined(QUANTUM_PAINTER_ENABLE)
        case PK_QCLR:
            if (pressed) {
                qp_log_clear();
            }
            return false;
#endif

#if defined(KEYLOG_ENABLE)
        case PK_KLOG:
            if (pressed) {
                keylog_enabled = !keylog_enabled;
            }
            return false;
#endif

        case PK_LOG:
            if (pressed) {
                step_level_for(UNKNOWN, !l_sft);
            }
            return false;

        case PK_SIZE:
            if (pressed) {
                int len = pretty_bytes(&str, get_flash_size());
                logging(UNKNOWN, LOG_INFO, "Binary takes %s", str.ptr - len);
            }
            return false;

        default:
            break;
    }

    return true;
}
