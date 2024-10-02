// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Abstract away some compiler-specific features.
 *
 * .. note::
 *   So far, just a few of GCC's attributes.
 *
 * .. seealso::
 *   `GCC Docs <https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html>`_
 *
 *   `ARM Docs <https://developer.arm.com/documentation/dui0491/i/Compiler-specific-Features/Function-attributes>`_
 *
 * .. attention::
 *   Documenting this file is not really feasible, take a look at its source.
 */

// -- barrier--

#pragma once

// re-export QMK utils
#include <quantum/util.h>

// base for other attributes
#ifndef _ATTR
#    define _ATTR(x) __attribute__((x))
#endif

// define an item as an alias to another one
#ifndef ALIAS
#    define ALIAS(x) _ATTR(alias(x))
#endif

// function pretty much just operates on inputs
#ifndef CONST
#    define CONST _ATTR(const)
#endif

// function is called before entering main, arg is priority
#ifndef CONSTRUCTOR
#    define CONSTRUCTOR(x) _ATTR(constructor(x))
#endif

// function is deprecated
#ifndef DEPRECATED
#    define DEPRECATED(x) _ATTR(deprecated(x))
#endif

// UNSUPPORTED
// indirect function, eg:
// void *my_memcpy(void *dst, const void *src, size_t len) { return dst; };
// static void *(*resolve_memcpy(void))(void *, const void *, size_t) { return my_memcpy; } // we will just always select this routine
// ISR("resolve_memcpy") void *memcpy(void *dst, const void *src, size_t len);
#ifndef IFUNC
#    define IFUNC(x) _ATTR(ifunc(x))
#endif

// function is an Interrupt Serving Routing
#ifndef INTERRUPT
#    define INTERRUPT _ATTR(interrupt)
#endif

// condition is likely, may get some optimization
#ifndef LIKELY
#    define LIKELY(x) __builtin_expect(!!(x), 1)
#endif

// UNSUPPORTED
// function does not use the argument
#ifndef NO_ACCESS
#    define NO_ACCESS(x) _ATTR(access(none, x))
#endif

// function arg can't be NULL
#ifndef NON_NULL
#    define NON_NULL(x) _ATTR(nonnull(x))
#endif

// function does not return
#ifndef NORETURN
#    define NORETURN _ATTR(noreturn)
#endif

// prevent padding
#ifndef PACKED
#    define PACKED _ATTR(packed)
#endif

// printf-like function (format + va_args), hardcoded assumption that they are correlative
#ifndef PRINTF
#    define PRINTF(x) _ATTR(format(printf, x, x + 1))
#endif

// function return does not depend on external state
#ifndef PURE
#    define PURE _ATTR(pure)
#endif

// mark a pointer argument as read-only
// second parameter would mean that another argument marks how much bytes will be read
#ifndef READ_ONLY
#    define READ_ONLY(args...) _ATTR(access(read_only, args))
#endif

// mark a pointer argument as read+write
// second parameter would mean that another argument marks how much bytes will be read/written
#ifndef READ_WRITE
#    define READ_WRITE(args...) _ATTR(access(read_write, args))
#endif

// function never returns null
#ifndef RETURN_NO_NULL
#    define RETURN_NO_NULL _ATTR(returns_nonnull)
#endif

// place item on the given section
#ifndef SECTION
#    define SECTION(x) _ATTR(section(x))
#endif

// function is not available (got removed)
#ifndef UNAVAILABLE
#    define UNAVAILABLE(x) _ATTR(unavailable(x))
#endif

// condition is unlikely, may get some optimization
#ifndef UNLIKELY
#    define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

// prevent compiler from error if the item isn't used
#ifndef UNUSED
#    define UNUSED _ATTR(unused)
#endif

// force compiler/linker to keep item in final binary
#ifndef USED
#    define USED _ATTR(used)
#endif

// mark a function's return value as must-be-used
#ifndef WARN_UNUSED
#    define WARN_UNUSED _ATTR(warn_unused_result)
#endif

// item can be overridden
#ifndef WEAK
#    define WEAK _ATTR(weak)
#endif

// item can be overridden
#ifndef WEAKREF
#    define WEAKREF(x) _ATTR(weakref(x))
#endif

// mark a pointer argument as write-only
// second parameter would mean that another argument marks how much bytes will be written
#ifndef WRITE_ONLY
#    define WRITE_ONLY(args...) _ATTR(access(write_only, args))
#endif
