// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Utilities on top of XAP.
 *
 * .. seealso::
 *    QMK's `documentation <https://github.com/qmk/qmk_firmware/blob/xap/docs/xap_0.3.0.md>`_ about XAP.
 *
 * .. caution::
 *   XAP is still work in progress.
 *
 *   `Pull Request <https://github.com/qmk/qmk_firmware/pull/13733>`_ for the feature.
 */

// -- barrier --

#pragma once

#include <stdint.h>

void xap_last_activity_update(void);

uint32_t xap_last_activity_time(void);
uint32_t xap_last_activity_elapsed(void);
