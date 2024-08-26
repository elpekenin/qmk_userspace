#pragma once

#include "elpekenin/utils/ld.h"

#define LD_BLOCK(x) \
    . = ALIGN(4); \
    LD_START(x) = .; \
    KEEP(*(SORT(LD_NAME(x).*))); \
    LD_END(x) = .;

SECTIONS {
   .elpekenin_funcs : ALIGN(4) {
      LD_BLOCK(pre_init)
      LD_BLOCK(post_init)
      LD_BLOCK(sendchar)
      LD_BLOCK(core1_init)
      LD_BLOCK(core1_loop)
      LD_BLOCK(deinit)
    } > flash1
}
