// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * based on Linux's kconfig.h
 *
 * used so that dead code is still analyzed by compiler and detect bugs (pre-processor would "comment" instead)
 *
 * NOTE: does not incur extra binary size because conditionals like `if (IS_DEFINED(FEATURE)) { feature_function() };`
 *       will be stripped away thanks to compiler/linker identifying the function will never be used
 *       otherwise we'd get an 'undefined symbol' error when trying to resolve the function name (it was not compiled)
 */

// -- barrier --

#pragma once

// generated file
#include "elpekenin/autoconf.h"

//
// internal
//

#define __PLACEHOLDER__1 0,
#define __SECOND_ARG(_, val, ...) val
#define __IS_DEFINED(x) __SECOND_ARG(x 1, 0)
#define _IS_DEFINED(x) __IS_DEFINED(__PLACEHOLDER__##x)

/**
 * Extras for "weird" use cases
 *     - `__PLACEHOLDER__TRUE` for `#define FLAG TRUE` when ChibiOS' `#define TRUE 1` not in scope
 *     - `__PLACEHOLDER__` for `#define FLAG`(no value)
 */
#define __PLACEHOLDER__TRUE 0,
#define __PLACEHOLDER__ 0,

//
// Public API
//

#define IS_DEFINED(x) _IS_DEFINED(x)
#define IS_ENABLED(x) IS_DEFINED(x##_ENABLE)
#define CM_ENABLED(x) IS_ENABLED(COMMUNITY_MODULE_##x)
