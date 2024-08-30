// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

typedef struct allocator_t allocator_t;

typedef void * (*calloc_fn)(allocator_t *allocator, size_t nmemb, size_t size);
typedef void   (*free_fn)(allocator_t *allocator, void *ptr);
typedef void * (*malloc_fn)(allocator_t *allocator, size_t size);
typedef void * (*realloc_fn)(allocator_t *allocator, void *ptr, size_t size);

struct allocator_t {
    calloc_fn  calloc;
    free_fn    free;
    malloc_fn  malloc;
    realloc_fn realloc;
    // ---
    size_t      used;
    const char *name;
    void *      arg;  // arbitrary config used by allocated, eg a pool in ChibiOS
} PACKED;

extern allocator_t c_runtime_allocator;
allocator_t *get_default_allocator(void);

void *calloc_with(allocator_t *allocator, size_t nmemb, size_t size);
void  free_with(allocator_t *allocator, void *ptr);
void *malloc_with(allocator_t *allocator, size_t total_size);
void *realloc_with(allocator_t *allocator, void *ptr, size_t size);

size_t get_used_heap(void);
const allocator_t **get_known_allocators(int8_t *n);

#if defined(PROTOCOL_CHIBIOS)
#include <ch.h>

_Static_assert(CH_CFG_USE_MEMCORE == TRUE, "Enable ChibiOS core allocator");
#include <chmemcore.h>
extern allocator_t ch_core_allocator;


#if CH_CFG_USE_MEMPOOLS == TRUE
#include <chmempools.h>
allocator_t new_ch_pool_allocator(memory_pool_t *pool, const char *name);
#endif

#if CH_CFG_USE_HEAP == TRUE
#include <chmemheaps.h>
allocator_t new_ch_heap_allocator(memory_heap_t *heap, const char *name);
#endif
#endif
