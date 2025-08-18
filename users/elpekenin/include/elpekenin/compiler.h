// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifndef __always_inline
#    define __always_inline __attribute__((__always_inline__))
#endif

#ifndef __interrupt
#    define __interrupt __attribute__((__interrupt__))
#endif

#ifndef __noinit
#    define __noinit __attribute__((section(".no_init")))
#endif

#ifndef __noreturn
#    define __noreturn __attribute__((__noreturn__))
#endif

#ifndef __unreachable
#    define __unreachable() __builtin_unreachable()
#endif

#ifndef __unused
#    define __unused __attribute__((__unused__))
#endif

#ifndef __warn_unused
#    define __warn_unused __attribute__((__warn_unused_result__))
#endif

#ifndef __weak_symbol
#    define __weak_symbol __attribute__((__weak__))
#endif
