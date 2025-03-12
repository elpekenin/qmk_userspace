// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Get some identification about the version of the code running on your board.
 */

/**
 * ----
 */

// -- barrier --

#pragma once

#include "elpekenin/utils/compiler.h"

#include "generated/features.h"

/**
 * How long the buffer for commit's text is.
 */
#define COMMIT_TEXT_LEN 12

/**
 * Information about a build.
 */
typedef struct PACKED {
    /**
     * Short commit hash, and marker if tree is dirty.
     *
     * As granted by QMK's ``version.h``
     */
    char commit[COMMIT_TEXT_LEN];

    /**
     * Set of features enabled on this build.
     *
     * Granted by my ``generated/features.h``.
     */
    enabled_features_t features;
} build_info_t;

/**
 * Get current binary's information.
 */
build_info_t get_build_info(void);

/**
 * Set the current binary's information.
 *
 * This is used so that slave side has up-to-date information,
 * without reflashing both halves. Data is sent over.
 */
void set_build_info(build_info_t new_build_info);
