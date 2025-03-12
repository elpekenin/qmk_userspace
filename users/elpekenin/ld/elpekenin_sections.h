#pragma once

#include "elpekenin/ld.h"

/*
 * NOTE: clang thinks this is a header, due to .h extension
 * and formats this is a way that breaks the linker
 */
// clang-format off

#define LD_BLOCK(x)              \
    . = ALIGN(4);                \
    LD_START(x) = .;             \
    KEEP(*(SORT(LD_NAME(x).*))); \
    LD_END(x) = .;

SECTIONS {
    .elpekenin_funcs : ALIGN(4) {
        X_SECTIONS(LD_BLOCK)
    } > flash1
}
