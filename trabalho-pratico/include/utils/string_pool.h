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

#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <inttypes.h>

/**
 * @file    string_pool.h
 * @brief   An allocator for strings.
 * @details A string pool is implemented as a dynamic arrays of pointers to static arrays (called
 *          blocks). Therefore, when an item is added and there's no space left in the current
 *          blocks, there's no need for a large reallocation, and thus there are no memory peaks
 *          nor the risk of dangling pointers, as a new block simply needs to be allocated.
 *
 * @anchor string_pool_examples
 * ### Examples
 *
 * The following example allocates string in a string pool. In practice, `TEST_POOL_BLOCK_SIZE`
 * should be way larger. It's only small to demonstrate how this string pool can handle strings
 * larger than its block size.
 *
 * ```c
 * #include <stdio.h>
 * #include <stdlib.h>
 *
 * #include "utils/string_pool.h"
 *
 * // Number of characters in a pool block. In practice, this should be way larger.
 * #define TEST_POOL_BLOCK_SIZE 32
 *
 * // Number of pool items to be allocated
 * #define TEST_NUM_ITEMS 100000
 *
 * int main(void) {
 *     string_pool_t *pool = string_pool_create(TEST_POOL_BLOCK_SIZE);
 *
 *     const char *long_string =
 *         "This string is longer than a single block, but the pool can still handle it!";
 *     const char *short_string = "Hello, world!";
 *
 *     char *allocated[TEST_NUM_ITEMS] = {0};
 *     for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
 *         int r = rand() % 2 == 1; // Choose randomly between the small and large string
 *         allocated[i] = string_pool_put(pool, r ? short_string : long_string);
 *
 *         if (!allocated[i]) {
 *             fputs("Allocation error!\n", stderr);
 *             string_pool_free(pool);
 *             return 1;
 *         }
 *     }
 *
 *     for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
 *         printf("%s\n", allocated[i]);
 *     }
 *
 *     string_pool_free(pool);
 *     return 0;
 * }
 * ```
 *
 * Instead of using ::string_pool_put, you could first allocate space for a string with
 * ::string_pool_allocate, and then copy its contents to the returned pointer.
 */

/**
 * @brief   An allocator for strings.
 * @details A string pool is implemented as a dynamic arrays of pointers to static arrays (called
 *          blocks). Therefore, when an item is added and there's no space left in the current
 *          blocks, there's no need for a large reallocation, and thus there are no memory peaks
 *          nor the risk of dangling pointers, as a new block simply needs to be allocated.
 */
typedef struct string_pool string_pool_t;

/**
 * @brief   Creates a string pool.
 * @details The returned value is owned by the caller, and should be freed with ::string_pool_free.
 *
 * @param block_capacity The number of characters in each block in the pool.
 *
 * @return The allocated pool, or `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_examples).
 */
string_pool_t *string_pool_create(size_t block_capacity);

/**
 * @brief   Allocates space for a string in the pool.
 * @details That string does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::string_pool_free.
 *
 * @param pool Pool to allocate the string in.
 * @param length Lenght of the string (not including null terminator).
 *
 * @return The pointer to the allocated string, `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_examples).
 */
char *string_pool_allocate(string_pool_t *pool, size_t length);

/**
 * @brief Allocates space and copies a string to a string pool.
 * @details That string does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::string_pool_free.
 *
 * @param pool Pool to allocate the string in.
 * @param str  String to be copied to the pool.
 *
 * @return The pointer to the allocated string, `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_examples).
 */
char *string_pool_put(string_pool_t *pool, const char *str);

/**
 * @brief   Removes all strings from @p pool.
 * @details Keep in mind that all values allocated using @p pool will no longer be valid. This
 *          should only be used when you want to allocate temporary data and free it many times
 *          over. This method allows you to reduce the number of pool creations (thus, allocations).
 *
 * @p pool String pool to have all its strings removed from it.
 */
void string_pool_empty(string_pool_t *pool);

/**
 * @brief Frees memory allocated by a string pool.
 * @param pool String pool to be freed.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_examples).
 */
void string_pool_free(string_pool_t *pool);

#endif
