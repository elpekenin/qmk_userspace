/*
 * Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
 * SPDX-License-Identifier: GPL-2.0-or-later

 * WARNING: Keep this pure pre-processor, is used on both C and LD
*/

#pragma once

#define LD_START(x) __elpekenin_##x##_base__
#define LD_END(x) __elpekenin_##x##__end__
#define LD_NAME(x) .elpekenin_##x

#define LD_BLOCK(x)              \
    . = ALIGN(4);                \
    LD_START(x) = .;             \
    KEEP(*(SORT(LD_NAME(x).*))); \
    LD_END(x) = .;

/**
 * Apply a function-like macro (``X``) to every custom section.
 */
#define X_SECTIONS(X) \
    X(pre_init)       \
    X(post_init)      \
    X(core1_init)     \
    X(core1_loop)     \
    X(deinit)         \
    X(sendchar)
