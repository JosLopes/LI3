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
#include <stddef.h>

/**
 * @file    pool.h
 * @brief   A pool allocator for structures of the same size.
 * @details A pool is implemented as a dynamic arrays of pointers to static arrays (called blocks).
 *          Therefore, when an item is added and there's no space left in the current topmost block,
 *          there's no need for a large reallocation, and thus there are no memory peaks nor
 *          the risk of dangling pointers, as a new block simply needs to be allocated.
 *
 * @anchor pool_examples
 * ### Examples
 *
 * The following example is not an illustrative use case for this module. It should be used with
 * large `structs` as items, and allocated pointers are to be used in complex data structures, such
 * as hash tables. The following `int *` array example is not a good use case (a simple `int` array
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
 *
 * Now, suppose that, instead of the `allocated` array, we had a more sensible data structure we
 * added pool-allocated items to, such as a hash table. Iterating through it wouldn't be very wise
 * when you can iterate through the pool:
 *
 * ```c
 * int callback(void *user_data, const void *item) {
 *     (void) user_data;
 *     printf("%d\n", * (const int *) item);
 *     return 0; // Another value can be returned to stop iteration
 * }
 *
 * // In main.c
 * pool_iter(pool, callback, NULL);
 * ```
 */

/** @brief A pool allocator for structures of the same size. */
typedef struct pool pool_t;

/**
 * @brief   Callback type for pool iterations.
 * @details Method called by ::pool_iter for every item in a ::pool_t.
 *
 * @param user_data Argument passed to ::pool_iter, that is passed to every callback, so that this
 *                  method can change the program's state.
 * @param item      Item in the pool. You must convert it to its correct type (type provided to
 *                  ::pool_create).
 *
 * @return `0` on success, or any other value to order pool iteration to stop.
 */
typedef int (*pool_iter_callback_t)(void *user_data, const void *item);

/**
 * @brief   Creates a pool from the size of its elements.
 * @details Unless you are using opaque types with a `*_sizeof` function exposed
 *          (e.g.: ::user_sizeof), **use ::pool_create instead.** The returned value is owned by the
 *          caller, and should be freed with ::pool_free.
 *
 * @param item_size      The size (in bytes) of the type of item to be allocated in this pool. For
 *                       example, this should be `sizeof(int)` for a pool of `int`s.
 * @param block_capacity The size (in items) of each block of the pool. Tune this parameter for best
 *                       performance / memory usage balance.
 *
 * @return The newly created pool, or `NULL` on allocation failure.
 */
pool_t *pool_create_from_size(size_t item_size, size_t block_capacity);

/**
 * @brief   Creates a pool.
 * @details The returned value is owned by the caller, and should be freed with ::pool_free.
 *
 * @param type           The type of the item in the pool (for example, `int`).
 * @param block_capacity A `size_t` with the number of items in each pool block. Tune this parameter
 *                       for best performance / memory usage balance.
 *
 * @return A pointer to a ::pool_t, or `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_create(type, block_capacity) pool_create_from_size(sizeof(type), block_capacity)

/**
 * @brief   Allocates space for an item in a pool. **Use ::pool_alloc_item instead.**
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free. However, keep in mind that the pool must oulive the allocated item. This
 *          method needs to be exposed so that the ::pool_alloc_item macro works.
 *
 * @param pool Pool in which the item will be allocated.
 *
 * @return The pointer to the allocated item, `NULL` on failure.
 */
void *__pool_alloc_item(pool_t *pool);

/**
 * @brief   Allocates space for an item in a pool.
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free. However, keep in mind that the pool must oulive the allocated item.
 *
 * @param type Type of the items in the pool (e.g.: `int`). It's expected to be the same type used
 *             in ::pool_create. Otherwise, this will result in undefined behavior.
 * @param pool A `pool_t *` to allocate the item in.
 *
 * @return The pointer to the allocated item, `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_alloc_item(type, pool) ((type *) __pool_alloc_item(pool))

/**
 * @brief   Allocates space for @p n contiguous items in a pool (an array of items). **Use
 *          ::pool_alloc_items instead.**
 * @details That array of items does not need to be `free`'d, as that's done when @p pool itself is
 *          freed in ::pool_free. However, keep in mind that the pool must oulive the allocated
 *          items. This method needs to be exposed so that the ::pool_alloc_items macro works.
 *
 * @param pool Pool in which the items will be allocated.
 * @param n    Number of items to be allocated.
 *
 * @return The pointer to the array of allocated items, `NULL` on failure.
 */
void *__pool_alloc_items(pool_t *pool, size_t n);

/**
 * @brief   Allocates space for @p n contiguous items in a pool (an array of items).
 * @details That array of items does not need to be `free`'d, as that's done when @p pool itself is
 *          freed in ::pool_free. However, keep in mind that the pool must oulive the allocated
 *          items.
 *
 * @param type Type of the items in the pool. It's expected to be the same type used in
 *             ::pool_create. Otherwise, this will result in undefined behavior.
 * @param pool A `pool_t *` in which the items will be allocated.
 * @param n    Number of items to be allocated.
 *
 * @return The pointer to the array of allocated items, `NULL` on failure.
 */
#define pool_alloc_items(type, pool, n) ((type *) __pool_alloc_items(pool, n))

/**
 * @brief   Adds an item to the pool, by allocating space for it and copying it there. **Use
 *          ::pool_put_item instead.**
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free. However, keep in mind that the pool must oulive the allocated item. This
 *          method needs to be exposed so that the ::pool_put_item macro works.
 *
 * @param pool          Pool to add the item to.
 * @param item_location Location of the item to be allocated and copied. It must be a
 *                      `const type *`, where `type` is the type provided to ::pool_create.
 *                      Otherwise, this will result in undefined behavior.
 *
 * @return The pointer to the allocated and copied item, `NULL` on failure.
 */
void *__pool_put_item(pool_t *pool, const void *item_location);

/**
 * @brief   Adds an item to the pool, by allocating space for it and copying it there.
 * @details That item does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::pool_free. However, keep in mind that the pool must oulive the allocated item.
 *
 * @param type          Type of the items in the pool. It's expected to be the same type used in
 *                      ::pool_create. Otherwise, this will result in undefined behavior.
 * @param pool          A `pool_t *` to add the item to.
 * @param item_location Pointer to the item to be allocated and copied. It must be a `const type *`,
 *                      where `type` is the type provided to ::pool_create. Otherwise, this will
 *                      result in undefined behavior.
 *
 * @return The pointer to the allocated and copied item, `NULL` on failure. The returned value will
 *         result from a **shallow copy** of @p item_location to the pool.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
#define pool_put_item(type, pool, item_location) ((type *) __pool_put_item(pool, item_location))

/**
 * @brief   Adds an array of items to the pool, by allocating space for it and copying it there.
 *          **Use ::pool_put_items instead.**
 * @details That array of items does not need to be `free`'d, as that's done when @p pool itself is
 *          freed in ::pool_free. However, keep in mind that the pool must oulive the allocated
 *          items. This method needs to be exposed so that the ::pool_put_items macro works.
 *
 *          Also, due to the inner workings of this method, **pool iterations won't work** if this
 *          function is ever called for a given pool.
 *
 * @param pool           Pool to add the item to.
 * @param items_location Location of the items to be allocated and copied. It must be a
 *                       `const type *`, where `type` is the type provided to ::pool_create.
 *                       Otherwise, this will result in undefined behavior.
 * @param n              Number of items in @p items_location.
 *
 * @return The pointer to the allocated and copied items, `NULL` on failure.
 */
void *__pool_put_items(pool_t *pool, const void *items_location, size_t n);

/**
 * @brief   Adds an array of items to the pool, by allocating space for it and copying it there.
 * @details That array of items does not need to be `free`'d, as that's done when @p pool itself is
 *          freed in ::pool_free. However, keep in mind that the pool must oulive the allocated
 *          items.
 *
 *          Also, due to the inner workings of this method, **pool iterations won't work** if this
 *          function is ever called for a given pool.
 *
 * @param pool           Pool to add the item to.
 * @param items_location Location of the items to be allocated and copied. It must be a `type *`,
 *                       where `type` is the type provided to ::pool_create. Otherwise, this will
 *                       result in undefined behavior.
 * @param n              Number of items in @p items_location.
 *
 * @return The pointer to the allocated and copied items, `NULL` on failure.
 */
#define pool_put_items(type, pool, items_location, n)                                              \
    ((type *) __pool_put_items(pool, items_location, n))

/**
 * @brief Value returned by ::pool_iter due to failure, because ::pool_put_items has been called
 *        before.
 */
#define POOL_ITER_RET_ADDED_ARRAY -1

/**
 * @brief Iterates through every item in the pool, calling @p callback for each one.
 *
 * @param pool      Pool to iterate thorugh.
 * @param callback  Method called for every item stored in @p pool.
 * @param user_data Pointer to be passed to every @p callback, so that it can modify the program's
 *                  state.
 *
 * @return The return value of the last-called @p callback (values other than `0` order iteration
 *         to stop). ::POOL_ITER_RET_ADDED_ARRAY can be returned if ::pool_put_items has been
 *         called before.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
int pool_iter(const pool_t *pool, pool_iter_callback_t callback, void *user_data);

/**
 * @brief   Removes all elements from @p pool.
 * @details Keep in mind that all values allocated using @p pool will no longer be valid (this will
 *          essentially `free` those pointers). This is meant to be used when you want to allocate
 *          temporary data and free it many times over. This method allows you to reduce the number
 *          of pool creations (thus, allocations).
 *
 * @p pool Pool to have all its elements removed from it.
 */
void pool_empty(pool_t *pool);

/**
 * @brief Frees memory allocated by a pool.
 * @param pool Pool to be freed.
 *
 * #### Examples
 * See [the header file's documentation](@ref pool_examples).
 */
void pool_free(pool_t *pool);

#endif
