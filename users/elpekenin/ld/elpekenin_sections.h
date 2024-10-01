#pragma once

#include "elpekenin/utils/ld.h"

SECTIONS {
    .elpekenin_funcs : ALIGN(4) {
        X_SECTIONS(LD_BLOCK)
    } > flash1
}
