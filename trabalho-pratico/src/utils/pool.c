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
 * @struct pool_block_t
 * @brief A static array with a fixed size, many of which form a ::pool_t.
 *
 * @var pool_block_t::data
 *   @brief Data in the pool block.
 * @var pool_block_t::used
 *   @brief Number of items already in the block.
 */
typedef struct {
    void  *data;
    size_t used;
} pool_block_t;

/**
 * @struct pool_t
 * @brief  Pool allocator for objects of the same size.
 *
 * @var pool_t::blocks
 *   @brief List of blocks (::pool_block_t) in the pool.
 * @var pool_t::block_list_capacity
 *   @brief Capacity (in blocks) of the dynamic vector in ::pool_t::blocks.
 * @var pool_t::block_list_length
 *   @brief Number of blocks in ::pool_t::blocks.
 * @var pool_t::item_size
 *   @brief Size (in bytes) of an item in the pool.
 * @var pool_t::block_capacity
 *   @brief Capacity of each pool block (in items).
 */
struct pool_t {
    pool_block_t *blocks;
    size_t        block_list_capacity;
    size_t        block_list_length;

    size_t item_size;
    size_t block_capacity;
};

/**
 * @brief Capacity of the initial block dynamic vector in a pool.
 */
#define POOL_INITAL_BLOCK_LIST_CAPACITY 32

/**
 * @brief Adds a new block to the pool.
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __pool_allocate_block(pool_t *pool) {
    if (pool->block_list_length == pool->block_list_capacity) {
        size_t new_capacity = pool->block_list_capacity * 2;

        pool_block_t *new_block_list = realloc(pool->blocks, new_capacity * sizeof(pool_block_t));
        if (!new_block_list) {
            return 1;
        }

        pool->block_list_capacity = new_capacity;
        pool->blocks              = new_block_list;
    }

    pool_block_t *block = pool->blocks + pool->block_list_length;
    block->used         = 0;
    block->data         = malloc(pool->item_size * pool->block_capacity);
    if (!block->data)
        return 1;

    pool->block_list_length++;
    return 0;
}

pool_t *__pool_create(size_t item_size, size_t block_capacity) {
    pool_t *pool = malloc(sizeof(struct pool_t));
    if (!pool)
        return NULL;

    pool->item_size      = item_size;
    pool->block_capacity = block_capacity;

    pool->block_list_length   = 0;
    pool->block_list_capacity = POOL_INITAL_BLOCK_LIST_CAPACITY;
    pool->blocks              = malloc(pool->block_list_capacity * sizeof(pool_block_t));
    if (!pool->blocks) {
        free(pool);
        return NULL;
    }

    if (__pool_allocate_block(pool)) { /* Out of memory */
        free(pool->blocks);
        free(pool);
        return NULL;
    }

    return pool;
}

void *__pool_alloc_item(pool_t *pool) {
    pool_block_t *top_block = pool->blocks + (pool->block_list_length - 1); /* pre: length >= 1 */
    if (top_block->used == pool->block_capacity) {
        if (__pool_allocate_block(pool)) {
            return NULL;
        }

        top_block = pool->blocks + (pool->block_list_length - 1);
    }

    void *retval = (uint8_t *) top_block->data + pool->item_size * top_block->used;
    top_block->used++;
    return retval;
}

void *__pool_put_item(pool_t *pool, void *item_location) {
    void *dest = __pool_alloc_item(pool);
    if (!dest)
        return NULL;

    memcpy(dest, item_location, pool->item_size);
    return dest;
}

void pool_free(pool_t *pool) {
    for (size_t i = 0; i < pool->block_list_length; ++i) {
        free(pool->blocks[i].data);
    }

    free(pool->blocks);
    free(pool);
}
