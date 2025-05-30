// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/keymap_extras/sendstring_spanish.h>
#include <quantum/process_keycode/process_rgb_matrix.h>
#include <quantum/quantum.h>

#include "elpekenin/autoconf_rt.h"
#include "elpekenin/keycodes.h"
#include "elpekenin/keylog.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/signatures.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/string.h"
#include "elpekenin/xap.h"

// compat: function must exist
#if CM_ENABLED(MEMORY)
#    include "elpekenin/memory.h"
#else
#    define get_flash_size() 0
#endif

static struct {
    // dont mind me, just bodging my way in  :)
    bool last_td_spc;
    bool keylog_active;
} global_state = {0};

bool apply_autocorrect(uint8_t backspaces, const char *str, char *typo, char *correct) {
    logging(LOG_WARN, "'%s' - '%s'", typo, correct);

    // regular handle
    if (!global_state.last_td_spc) {
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
    global_state.last_td_spc = false;

    switch (*keycode) {
        case TD_SPC:
            *keycode                 = KC_SPC; // make this look like a regular spacebar
            record->event.pressed    = false;  // trigger an extra backspace when corrected
            global_state.last_td_spc = true;
            break;

        case TD_Z:
            *keycode = KC_Z;
            break;

        default:
            break;
    }
    return process_autocorrect_default_handler(keycode, record, typo_buffer_size, mods);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    string_t str = str_new(15);

    if (IS_ENABLED(KEYLOG) && global_state.keylog_active) {
        keylog_process(keycode, record);
    }

    // log events over XAP
    if (IS_ENABLED(XAP)) {
        xap_keyevent(keycode, record);
    }

    if (IS_ENABLED(RGB_MATRIX) && IS_RGB_MATRIX_KEYCODE(keycode)) {
        if (record->event.pressed) {
            logging(LOG_INFO, "Used %s", get_keycode_string(keycode));
        }

        // no debugging here, noise in logs
        bool old            = debug_config.enable;
        debug_config.enable = false;

        bool ret = process_rgb_matrix(keycode, record);

        debug_config.enable = old;

        return ret;
    }

    if (!process_record_keymap(keycode, record)) {
        return false;
    }

    bool    pressed = record->event.pressed;
    uint8_t mods    = get_mods();
    bool    l_sft   = mods & MOD_BIT(KC_LSFT);

    switch (keycode) {
        case EE_CLR:
            // reset on slave too
            if (IS_DEFINED(SPLIT_KEYBOARD) && pressed) {
                reset_ee_slave();
            }
            return true;

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

        case PK_QCLR:
            if (IS_ENABLED(QP_LOG) && pressed) {
                qp_log_clear();
            }
            return false;

        case PK_KLOG:
            if (IS_ENABLED(KEYLOG) && pressed) {
                global_state.keylog_active ^= true;
            }
            return false;

        case PK_LOG:
            if (pressed) {
                step_logging_level(!l_sft);
            }
            return false;

        case PK_SIZE:
            if (CM_ENABLED(MEMORY) && pressed) {
                pretty_bytes(&str, get_flash_size());
                logging(LOG_INFO, "Binary takes %.*s", str.used, str.ptr);
            }
            return false;

        case PK_CONF:
            if (pressed) {
                const autoconf_setting_t *settings = get_autoconf_settings();
                for (size_t i = 0; i < autoconf_settings_count(); ++i) {
                    const autoconf_setting_t setting = settings[i];

                    printf("%s=", setting.name);
                    switch (setting.value.type) {
                        case AUTOCONF_INT:
                            printf("%d", setting.value.integer);
                            break;

                        case AUTOCONF_STRING:
                            printf("\"%s\"", setting.value.string);
                            break;
                    }
                    print("\n");
                }
            }
            return false;

        default:
            break;
    }

    return true;
}
