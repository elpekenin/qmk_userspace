// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// generated file
#include "elpekenin/autoconf.h"

// based on Linux's kconfig.h
//
// used so that dead code is still analyzed by compiler and detect bugs (pre-processor would "comment" instead)
//
// NOTE: does not incur extra binary size because conditionals like `if (IS_DEFINED(FEATURE)) { feature_function() };`
//       will be stripped away thanks to compiler/linker identifying the function will never be used
//       otherwise we'd get an 'undefined symbol' error when trying to resolve the function name (it was not compiled)
#define __PLACEHOLDER__1 0,
// NOTE: addition so that `#define FOO` also works (not only `#define FOO 1`)
#define __PLACEHOLDER__ 0,
#define __SECOND_ARG(_, val, ...) val
#define __IS_DEFINED(x) __SECOND_ARG(x 1, 0)
#define _IS_DEFINED(x) __IS_DEFINED(__PLACEHOLDER__##x)

#define IS_DEFINED(x) _IS_DEFINED(x)
#define IS_ENABLED(x) IS_DEFINED(x##_ENABLE)
#define CM_ENABLED(x) IS_ENABLED(COMMUNITY_MODULE_##x)
