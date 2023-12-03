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

#include "utils/pool.h"
#include "utils/string_pool.h"

/**
 * @struct string_pool
 * @brief  Pool allocator for strings.
 *
 * @var string_pool::pool
 *   @brief Standard pool, used to implement the string pool.
 */
struct string_pool {
    pool_t *pool;
};

string_pool_t *string_pool_create(size_t block_capacity) {
    string_pool_t *pool = malloc(sizeof(string_pool_t));
    if (!pool)
        return NULL;

    pool->pool = pool_create(char, block_capacity);
    if (!pool->pool) {
        free(pool);
        return NULL;
    }

    return pool;
}

char *string_pool_allocate(string_pool_t *pool, size_t length) {
    return pool_alloc_items(char, pool->pool, length + 1);
}

char *string_pool_put(string_pool_t *pool, const char *str) {
    return pool_put_items(char, pool->pool, str, strlen(str) + 1);
}

void string_pool_empty(string_pool_t *pool) {
    pool_empty(pool->pool);
}

void string_pool_free(string_pool_t *pool) {
    pool_free(pool->pool);
    free(pool);
}
