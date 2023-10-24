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

#ifndef POOL_H
#define POOL_H

#include <inttypes.h>

/**
 * @file    pool.h
 * @brief   A pool allocator for structure of the same size.
 * @details A pool is implemented as a dynamic arrays of pointers to static arrays (called blocks).
 *          Therefore, when an item is added and there's no space left in the current blocks,
 *          there's no need for a large reallocation, and thus there are no memory peaks nor
 *          the risk of dangling pointers, as a new block simply needs to be allocated.
 *
 * @anchor pool_examples
 * ### Examples
 *
 * The following example is not an illustrative use case for this module. It should be used with
 * large `structs` as items, and allocated pointers are to be used in complex data structures, such
 * as hashmaps. The following `int *` array example is not a good use case (a simple `int` array
 * would do), but it shows how to use the pool's methods.
 *
 * ```c
 * #include <stdio.h>
 * #include "utils/pool.h"
 *
 * // Number of items in a pool block
 * #define TEST_POOL_BLOCK_SIZE 1000
 *
 * // Number of pool items to be allocated
 * #define TEST_NUM_ITEMS 100000
 *
 * int main(void) {
 *     pool_t *pool = pool_create(int, TEST_POOL_BLOCK_SIZE);
 *
 *     int *allocated[TEST_NUM_ITEMS] = {0};
 *     for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
 *         allocated[i] = pool_put_item(int, pool, &i);
 *         if (!allocated[i]) {
 *             fputs("Allocation error!\n", stderr);
 *             pool_free(pool);
 *             return 1;
 *         }
 *     }
 *
 *     for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
 *         printf("%d\n", *allocated[i]);
 *     }
 *
 *     pool_free(pool);
 *     return 0;
 * }
 * ```
 *
 * ::pool_put_item could be replaced by an allocation, followed by modification of the value in the
 * returned pointer:
 *
 * ```c
 * allocated[i] = pool_alloc_int(int, pool);
 * *allocated[i] = i;
 * ```
 */

/**
 * @brief   A pool allocator.
 * @details A pool is implemented as a dynamic arrays of pointers to static arrays (called blocks).
 *          Therefore, when an item is added and there's no space left in the current blocks,
 *          there's no need for a large reallocation, and thus there are no memory peaks nor
 *          the risk of dangling pointers, as a new block simply needs to be allocated.
 */
typedef struct pool_t pool_t;

/**
 * @brief   Creates a pool from the size of its elements. **Use ::pool_create instead.**
 * @details This method needs to be exposed so that the ::pool_create macro works. The returned
 *          value is owned by the caller, and should be freed with ::pool_free.
 *
 * @param item_size      The size (in bytes) of the type of item to be allocated in this pool. For
 *                       example, this should be `sizeof(int)` for a pool of `int`s.
 * @param block_capacity The size (in items) of each block of the pool.
 *
 * @return The allocated pool, or `NULL` on failure.
 */
pool_t *__pool_create(size_t item_size, size_t block_capacity);

/**
 * @brief   Creates a pool.
 * @details The returned value is owned by the caller, and should be freed with ::pool_free.
 *
 * @param type The type of the item in the pool.
 * @param block_capacity A `size_t` with the number of items in each pool block.
 *
 * @return The allocated pool, or `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_create(type, block_capacity) __pool_create(sizeof(type), block_capacity)

/**
 * @brief   Allocates space for an item in a pool. **Use ::pool_alloc_item instead.**
 * @details This method needs to be exposed so that the ::pool_alloc_item macro works.
 *
 * @param pool Pool in which the item will be allocated.
 *
 * @return The pointer to the allocated item, `NULL` on failure.
 */
void *__pool_alloc_item(pool_t *pool);

/**
 * @brief   Allocates space for an item in a pool.
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free.
 *
 * @param type Type of the items in the pool. It's expected to be the same type used in
 *             ::pool_create. Otherwise, this will result in undefined behavior.
 * @param pool A `pool_t *` to allocate the item in.
 *
 * @return The pointer to the allocated item, `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_alloc_item(type, pool) ((type *) __pool_alloc_item(pool))

/**
 * @brief   Adds an item to the pool, by allocating space for it and copying it there. **Use
 *          ::pool_put_item instead.**
 * @details This method needs to be exposed so that the ::pool_put_item macro works. That item does
 *          not need to be `free`'d, as that's done when @p pool itself is freed in ::pool_free.
 *
 * @param pool Pool to add the item to.
 * @param item_location Location of the item to be allocated and copied. It must be a `type *`,
 *                      where `type` is the type provided to ::pool_create. Otherwise, this will
 *                      result in undefined behavior.
 *
 * @return The pointer to the allocated and copied item, `NULL` on failure.
 */
void *__pool_put_item(pool_t *pool, void *item_location);

/**
 * @brief   Adds an item to the pool, by allocating space for it and copying it there.
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free.
 *
 * @param type Type of the items in the pool. It's expected to be the same type used in
 *             ::pool_create. Otherwise, this will result in undefined behavior.
 * @param pool A `pool_t *` to add the item to.
 * @param item_location Location of the item to be allocated and copied. It must be a `type *`,
 *                      where `type` is the type provided to ::pool_create. Otherwise, this will
 *                      result in undefined behavior.
 *
 * @return The pointer to the allocated and copied item, `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_put_item(type, pool, item_location) ((type *) __pool_put_item(pool, item_location))

/**
 * @brief Frees memory allocated by a pool.
 * @param pool Pool to be freed.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
void pool_free(pool_t *pool);

#endif
