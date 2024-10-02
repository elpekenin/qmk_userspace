// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO: Find issue causing spam of log_success error msgs
//       Maybe a bug here, maybe a bug on scrolling_text API

#include <stdlib.h>

#include <quantum/logging/debug.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/shortcuts.h"

// *** Track heap usage ***

static size_t             n_known = 0;
static const allocator_t *known_allocators[100];

// TODO: per-allocator stats
typedef struct PACKED {
    allocator_t *allocator;
    void        *ptr;
    size_t       size;
} alloc_info_t;

// size is fixed to prevent the tracker to be dynamic (use malloc) itself
static alloc_info_t  alloc_info_buff[100] = {0};
static memory_pool_t alloc_info_pool;

static void alloc_pool_init(void) {
    chPoolObjectInit(&alloc_info_pool, sizeof(alloc_info_t), NULL);
    chPoolLoadArray(&alloc_info_pool, alloc_info_buff, ARRAY_SIZE(alloc_info_buff));
}
PEKE_PRE_INIT(alloc_pool_init, INIT_ALLOC);

const allocator_t **get_known_allocators(int8_t *n) {
    *n = n_known;
    return known_allocators;
}

size_t get_used_heap(void) {
    size_t used = 0;

    int8_t              n;
    const allocator_t **allocators = get_known_allocators(&n);

    for (int8_t i = 0; i < n; ++i) {
        const allocator_t *allocator = allocators[i];
        used += allocator->used;
    }

    return used;
}

static inline alloc_info_t *find_info(void *ptr) {
    for (alloc_info_t *info = alloc_info_buff; info < &alloc_info_buff[ARRAY_SIZE(alloc_info_buff)]; ++info) {
        if (info->ptr == ptr) {
            return info;
        }
    }

    return NULL;
}

static void memory_allocated(allocator_t *allocator, void *ptr, size_t size) {
    if (UNLIKELY(ptr == NULL)) {
        return;
    }

    bool insert = true;
    for (size_t i = 0; i < n_known; ++i) {
        if (known_allocators[i] == allocator) {
            insert = false;
            break;
        }
    }
    if (insert) {
        known_allocators[n_known++] = allocator;
    }

    alloc_info_t *info   = chPoolAlloc(&alloc_info_pool);
    bool          pushed = info != NULL;

    if (LIKELY(pushed)) {
        allocator->used += size;

        *info = (alloc_info_t){
            .allocator = allocator,
            .ptr       = ptr,
            .size      = size,
        };
    }

    log_success(pushed, ALLOC, "%s", __func__);
}

static void memory_freed(void *ptr) {
    if (UNLIKELY(ptr == NULL)) {
        return;
    }

    alloc_info_t *info   = find_info(ptr);
    bool          popped = info != NULL;

    if (LIKELY(popped)) {
        info->allocator->used -= info->size;
        chPoolFree(&alloc_info_pool, ptr);
    }

    log_success(popped, ALLOC, "%s", __func__);
}

static inline void *calloc_shim(allocator_t *allocator, size_t nmemb, size_t size) {
    return calloc(nmemb, size);
}

static inline void free_shim(allocator_t *allocator, void *ptr) {
    return free(ptr);
}

static inline void *malloc_shim(allocator_t *allocator, size_t size) {
    return malloc(size);
}

static inline void *realloc_shim(allocator_t *allocator, void *ptr, size_t size) {
    return realloc(ptr, size);
}

allocator_t c_runtime_allocator = {
    .calloc  = calloc_shim,
    .free    = free_shim,
    .malloc  = malloc_shim,
    .realloc = realloc_shim,
    .name    = "C runtime",
};

#if defined(PROTOCOL_CHIBIOS)
static void *ch_core_malloc(allocator_t *allocator, size_t size) {
    return chCoreAlloc(size);
}

allocator_t ch_core_allocator = {
    .malloc = ch_core_malloc,
    .name   = "ChibiOS core",
};

static void *manual_realloc(allocator_t *allocator, void *ptr, size_t new_size) {
    // no pointer, realloc is equivalent to malloc
    if (ptr == NULL) {
        return malloc_with(allocator, new_size);
    }

    // pointer and new size is 0, realloc is equivalent to free
    if (new_size == 0) {
        free_with(allocator, ptr);
        return NULL;
    }

    // find current size
    alloc_info_t *info = find_info(ptr);
    if (UNLIKELY(info == NULL)) {
        _ = logging(ALLOC, LOG_ERROR, "Could not find info for realloc");
        return NULL;
    }

    // big enough, just return the current address back
    size_t curr_size = info->size;
    if (curr_size >= new_size) {
        return ptr;
    }

    // actual realloc
    void *new_ptr = malloc_with(allocator, new_size);
    if (UNLIKELY(new_ptr == NULL)) {
        // no space for new allocation
        // return NULL and **do not** free old memory
        _ = logging(ALLOC, LOG_ERROR, "New size could not be allocated.");
        return NULL;
    }

    memcpy(new_ptr, ptr, curr_size);
    free_with(allocator, ptr);
    return new_ptr;
}

#    if CH_CFG_USE_MEMPOOLS == TRUE
static void ch_pool_free(allocator_t *allocator, void *ptr) {
    memory_pool_t *pool = (memory_pool_t *)allocator->arg;
    return chPoolFree(pool, ptr);
}

static void *ch_pool_malloc(allocator_t *allocator, size_t size) {
    memory_pool_t *pool    = (memory_pool_t *)allocator->arg;
    size_t         n_items = size / pool->object_size;

    // ensure we get asked for a single item's size
    if (n_items != 1 || n_items * pool->object_size != size) {
        _ = logging(ALLOC, LOG_ERROR, "size / pool_obj_size != 1");
        return NULL;
    }

    return chPoolAlloc(pool);
}

allocator_t new_ch_pool_allocator(memory_pool_t *pool, const char *name) {
    return (allocator_t){
        .free   = ch_pool_free,
        .malloc = ch_pool_malloc,
        .name   = name,
        .arg    = pool,
    };
}
#    endif

#    if CH_CFG_USE_HEAP == TRUE
static void ch_heap_free(allocator_t *allocator, void *ptr) {
    return chHeapFree(ptr);
}

static void *ch_heap_malloc(allocator_t *allocator, size_t size) {
    return chHeapAlloc((memory_heap_t *)allocator->arg, size);
}

allocator_t new_ch_heap_allocator(memory_heap_t *heap, const char *name) {
    return (allocator_t){
        .free    = ch_heap_free,
        .malloc  = ch_heap_malloc,
        .realloc = manual_realloc,
        .name    = name,
        .arg     = heap,
    };
}
#    endif
#endif

PURE allocator_t *get_default_allocator(void) {
    return &c_runtime_allocator;
}

// ^ allocators
// -----
// v convenience wrappers

static inline void __entry(const char *fn, allocator_t *allocator) {
    _ = logging(ALLOC, LOG_DEBUG, "Using %s.%s", allocator->name, fn);
}

static inline void *__bad_allocator(const char *fn, allocator_t *allocator) {
    _ = logging(ALLOC, LOG_ERROR, "There is no %s.%s", allocator->name, fn);
    return NULL;
}

static inline void __error(const char *fn) {
    _ = logging(ALLOC, LOG_ERROR, "Calling %s failed", fn);
}

NON_NULL(1) void *calloc_with(allocator_t *allocator, size_t nmemb, size_t size) {
    const char fn[] = "calloc";

    __entry(fn, allocator);

    if (allocator->calloc == NULL) {
        return __bad_allocator(fn, allocator);
    }

    void *ptr = allocator->calloc(allocator, nmemb, size);
    if (UNLIKELY(ptr == NULL)) {
        __error(fn);
    } else {
        memory_allocated(allocator, ptr, nmemb * size);
    }

    return ptr;
}

NON_NULL(1) void free_with(allocator_t *allocator, void *ptr) {
    const char fn[] = "free";

    __entry(fn, allocator);

    if (allocator->free == NULL) {
        __bad_allocator(fn, allocator);
        return;
    }

    allocator->free(allocator, ptr);
    memory_freed(ptr);
}

NON_NULL(1) void *malloc_with(allocator_t *allocator, size_t size) {
    const char fn[] = "malloc";

    __entry(fn, allocator);

    if (allocator->malloc == NULL) {
        return __bad_allocator(fn, allocator);
    }

    void *ptr = allocator->malloc(allocator, size);
    if (UNLIKELY(ptr == NULL)) {
        __error(fn);
    } else {
        memory_allocated(allocator, ptr, size);
    }

    return ptr;
}

NON_NULL(1) void *realloc_with(allocator_t *allocator, void *ptr, size_t size) {
    const char fn[] = "realloc";

    __entry(fn, allocator);

    if (allocator->realloc == NULL) {
        return __bad_allocator(fn, allocator);
    }

    void *new_ptr = allocator->realloc(allocator, ptr, size);
    if (UNLIKELY(new_ptr == NULL)) {
        __error(fn);
    } else {
        // maybe it already went thru free and got cleaned?
        memory_freed(ptr);
        memory_allocated(allocator, new_ptr, size);
    }

    return new_ptr;
}
