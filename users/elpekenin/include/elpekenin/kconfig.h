// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// generated file
#include "elpekenin/autoconf.h"

#define KCONF(x) CONFIG_##x
#define KCONF_ENABLED(x) IS_ENABLED(KCONF(x))
