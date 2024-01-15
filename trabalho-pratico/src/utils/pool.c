/*
 * Copyright 2023 Humberto Gomes, José Lopes, José Matos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "utils/pool.h"

/**
 * @file  pool.c
 * @brief Implementation of methods in include/utils/pool.h
 *
 * ### Examples
 * See [the header file's documentation](@ref pool_examples).
 */

/**
 * @struct pool
 * @brief  Pool allocator for objects of the same size.
 *
 * @var pool::blocks
 *     @brief Array of blocks (`uint8_t *`'s) in the pool.
 * @var pool::item_size
 *     @brief Size (in bytes) of an item in the pool.
 * @var pool::block_capacity
 *     @brief Capacity of each pool block (in items).
 * @var pool::top_block_used
 *     @brief Number of items already in the top block of the pool.
 * @var pool::can_iterate
 *     @brief If a pool can be iterated over (::pool_put_items hasn't been called).
 */
struct pool {
    GPtrArray *blocks;

    size_t item_size;
    size_t block_capacity;
    size_t top_block_used;

    int can_iterate;
};

/**
 * @brief Adds a new block to the top of the pool.
 * @param pool Pool to add block to.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __pool_allocate_block(pool_t *pool) {
    uint8_t *const block = malloc(pool->item_size * pool->block_capacity);
    if (!block)
        return 1;

    g_ptr_array_add(pool->blocks, block);
    pool->top_block_used = 0;
    return 0;
}

/**
 * @brief   Adds a new block to the middle of a pool, that will only be used for storing a single
 *          array of items.
 * @details Auxiliary method for ::__pool_alloc_items. Used when `n > pool->block_capacity`.
 *
 * @param pool Pool to add block to.
 * @param n    Number of elements to allocate.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __pool_allocate_single_use_block(pool_t *pool, size_t n) {
    uint8_t *const block = malloc(pool->item_size * n);
    if (!block)
        return 1;

    g_ptr_array_insert(pool->blocks, pool->blocks->len - 1, block);
    return 0;
}

pool_t *pool_create_from_size(size_t item_size, size_t block_capacity) {
    pool_t *const pool = malloc(sizeof(pool_t));
    if (!pool)
        return NULL;

    pool->blocks         = g_ptr_array_new_with_free_func(free);
    pool->item_size      = item_size;
    pool->block_capacity = block_capacity;
    pool->top_block_used = 0;
    pool->can_iterate    = 1;

    if (__pool_allocate_block(pool)) {
        g_ptr_array_unref(pool->blocks);
        free(pool);
        return NULL;
    }

    return pool;
}

void *__pool_alloc_item(pool_t *pool) {
    if (pool->top_block_used == pool->block_capacity) {
        if (__pool_allocate_block(pool))
            return NULL;
    }

    uint8_t *const retval = (uint8_t *) g_ptr_array_index(pool->blocks, pool->blocks->len - 1) +
                            pool->item_size * pool->top_block_used;
    pool->top_block_used++;
    return retval;
}

void *__pool_alloc_items(pool_t *pool, size_t n) {
    pool->can_iterate = 0;

    if (n > pool->block_capacity) { /* Very large array */
        if (__pool_allocate_single_use_block(pool, n))
            return NULL;

        return g_ptr_array_index(pool->blocks, pool->blocks->len - 2);
    } else {
        const size_t block_left = pool->block_capacity - pool->top_block_used;
        if (n > block_left)
            if (__pool_allocate_block(pool))
                return NULL;

        uint8_t *retval = (uint8_t *) g_ptr_array_index(pool->blocks, pool->blocks->len - 1) +
                          pool->item_size * pool->top_block_used;
        pool->top_block_used += n;
        return retval;
    }
}

void *__pool_put_item(pool_t *pool, const void *item_location) {
    void *const dest = __pool_alloc_item(pool);
    if (!dest)
        return NULL;

    memcpy(dest, item_location, pool->item_size);
    return dest;
}

void *__pool_put_items(pool_t *pool, const void *items_location, size_t n) {
    void *const dest = __pool_alloc_items(pool, n);
    if (!dest)
        return NULL;

    memcpy(dest, items_location, pool->item_size * n);
    return dest;
}

int pool_iter(const pool_t *pool, pool_iter_callback_t callback, void *user_data) {
    if (!pool->can_iterate)
        return POOL_ITER_RET_ADDED_ARRAY;

    for (size_t i = 0; i < pool->blocks->len; ++i) {
        const uint8_t *const block = g_ptr_array_index(pool->blocks, i);
        const size_t         item_count =
            i == pool->blocks->len - 1 ? pool->top_block_used : pool->block_capacity;

        for (size_t j = 0; j < item_count; ++j) {
            const void *const item   = block + pool->item_size * j;
            const int         retval = callback(user_data, item);

            if (retval)
                return retval;
        }
    }

    return 0;
}

void pool_empty(pool_t *pool) {
    g_ptr_array_set_size(pool->blocks, 1);
    pool->top_block_used = 0;
    pool->can_iterate    = 1;
}

void pool_free(pool_t *pool) {
    g_ptr_array_unref(pool->blocks);
    free(pool);
}
