/*
 * Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
 * SPDX-License-Identifier: GPL-2.0-or-later

 * WARNING: Keep this pure pre-processor, is used on both C and LD
*/

#pragma once

#define LD_START(x) __elpekenin_##x##_base__
#define LD_END(x) __elpekenin_##x##__end__
#define LD_NAME(x) .elpekenin_##x