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

/**
 * @file  string_pool.c
 * @brief Implementation of methods in include/utils/string_pool.h
 *
 * ### Examples
 * See [the header file's documentation](@ref string_pool_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "utils/string_pool.h"

/**
 * @struct string_pool_block_t
 * @brief A static array with a fixed size, many of which form a ::string_pool_t.
 *
 * @var string_pool_block_t::data
 *   @brief Data in the pool block.
 * @var string_pool_block_t::used
 *   @brief Number of characters already in the block.
 */
typedef struct {
    void  *data;
    size_t used;
} string_pool_block_t;

/**
 * @struct string_pool_t
 * @brief  Pool allocator for strings.
 *
 * @var string_pool_t::blocks
 *   @brief List of blocks (::pool_block_t) in the pool.
 * @var string_pool_t::block_list_capacity
 *   @brief Capacity (in blocks) of the dynamic vector in ::string_pool_t::blocks.
 * @var string_pool_t::block_list_length
 *   @brief Number of blocks in ::string_pool_t::blocks.
 * @var string_pool_t::block_capacity
 *   @brief Capacity of each pool block (in characters).
 */
struct string_pool_t {
    string_pool_block_t *blocks;
    size_t               block_list_capacity;
    size_t               block_list_length;

    size_t block_capacity;
};

/**
 * @brief Capacity of the initial block dynamic vector in a pool.
 */
#define STRING_POOL_INITAL_BLOCK_LIST_CAPACITY 32

/**
 * @brief Reallocates the list of blocks, if that is necessary to add another block.
 * @param pool Pool to modify, if that's required.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __string_pool_reallocate_if_needed(string_pool_t *pool) {
    if (pool->block_list_length == pool->block_list_capacity) {
        size_t new_capacity = pool->block_list_capacity * 2;

        string_pool_block_t *new_block_list =
            realloc(pool->blocks, new_capacity * sizeof(string_pool_block_t));
        if (!new_block_list)
            return 1;

        pool->block_list_capacity = new_capacity;
        pool->blocks              = new_block_list;
    }

    return 0;
}

/**
 * @brief Adds a new block to a string pool, that can fit multiple strings.
 *
 * @param pool Pool to add a block to.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __string_pool_allocate_standard_block(string_pool_t *pool) {
    if (__string_pool_reallocate_if_needed(pool))
        return 1;

    string_pool_block_t *block = pool->blocks + pool->block_list_length;
    block->used                = 0;
    block->data                = malloc(pool->block_capacity);
    if (!block->data)
        return 1;

    pool->block_list_length++;
    return 0;
}

/**
 * @brief Adds a new block to a string pool, made to fit a single string longer than a standard
 *        block.
 *
 * @param pool   Pool to add a block to.
 * @param lenght Length of the string to be allocated.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __string_pool_allocate_single_string_block(string_pool_t *pool, size_t length) {
    if (__string_pool_reallocate_if_needed(pool))
        return 1;

    string_pool_block_t *block = pool->blocks + pool->block_list_length;
    block->used                = pool->block_capacity; /* Fake full block */
    block->data                = malloc(length + 1);
    if (!block->data)
        return 1;

    pool->block_list_length++;
    return 0;
}

string_pool_t *string_pool_create(size_t block_capacity) {
    string_pool_t *pool = malloc(sizeof(struct string_pool_t));
    if (!pool)
        return NULL;
    pool->block_capacity = block_capacity;

    pool->block_list_length   = 0;
    pool->block_list_capacity = STRING_POOL_INITAL_BLOCK_LIST_CAPACITY;
    pool->blocks              = malloc(pool->block_list_capacity * sizeof(string_pool_block_t));
    if (!pool->blocks) {
        free(pool);
        return NULL;
    }

    if (__string_pool_allocate_standard_block(pool)) { /* Out of memory */
        free(pool->blocks);
        free(pool);
        return NULL;
    }

    return pool;
}

char *string_pool_allocate(string_pool_t *pool, size_t length) {
    string_pool_block_t *top_block =
        pool->blocks + (pool->block_list_length - 1); /* pre: length >= 1 */

    if (length + 1 > pool->block_capacity) { /* Very long string */
        if (__string_pool_allocate_single_string_block(pool, length)) {
            return NULL;
        }

        /* Keep previous block on top (use any remaining space in the future) */
        string_pool_block_t tmp                   = pool->blocks[pool->block_list_length - 1];
        pool->blocks[pool->block_list_length - 1] = pool->blocks[pool->block_list_length - 2];
        pool->blocks[pool->block_list_length - 2] = tmp;

        return pool->blocks[pool->block_list_length - 2].data;
    }

    /* Normal strings */
    size_t block_left = pool->block_capacity - top_block->used;
    if (length + 1 > block_left) {
        if (__string_pool_allocate_standard_block(pool)) {
            return NULL;
        }

        top_block = pool->blocks + (pool->block_list_length - 1);
    }

    void *retval = (uint8_t *) top_block->data + top_block->used;
    top_block->used += length + 1;
    return retval;
}

char *string_pool_put(string_pool_t *pool, const char *str) {
    size_t length    = strlen(str);
    char  *allocated = string_pool_allocate(pool, length);
    if (!allocated)
        return NULL;

    memcpy(allocated, str, length + 1);
    return allocated;
}

void string_pool_free(string_pool_t *pool) {
    for (size_t i = 0; i < pool->block_list_length; ++i) {
        free(pool->blocks[i].data);
    }

    free(pool->blocks);
    free(pool);
}
