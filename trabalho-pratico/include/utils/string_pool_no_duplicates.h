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

#ifndef STRING_POOL_NO_DUPLICATES
#define STRING_POOL_NO_DUPLICATES

/**
 * @file    string_pool_no_duplicates.h
 * @brief   An allocator for strings subject to repetition, of which only one copy will be
 *          allocated.
 * @details Allocations are slightly slower than on a string pool, but memroy usage should improve
 *          by a lot if a string is repeated many times over.
 *
 * @anchor string_pool_no_duplicates_examples
 * ### Examples
 *
 * The following example allocates multiple strings in a string pool without duplicates, and every
 * new pointer that's returned from the allocator is stored in an array. In the end, the program
 * should only print `n` strings, `n` being the number of different strings.
 *
 * In practice, `TEST_POOL_BLOCK_SIZE` should be way larger. It's only small to demonstrate how this
 * string pool can handle strings larger than its block size.
 *
 * ```c
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include "utils/string_pool_no_duplicates.h"
 *
 * // In practice, this number should be way larger.
 * #define TEST_POOL_BLOCK_SIZE 16
 * #define TEST_NUM_OF_PUTS     10
 *
 * int main(void) {
 *    string_pool_no_duplicates_t *no_dups_pool =
 *        string_pool_no_duplicates_create(TEST_POOL_BLOCK_SIZE);
 *
 *    const char *string_0 = "Very creative string!";
 *    const char *string_1 = "Hello, world!";
 *
 *    const char *allocated[TEST_NUM_OF_PUTS] = {0};
 *    for (size_t i = 0; i < TEST_NUM_OF_PUTS; ++i) {
 *        int r = rand() % 2; // Choose between string or string_1.
 *        const char *temp = string_pool_no_duplicates_put(no_dups_pool, r ? string_0 : string_1);
 *        if (!temp) {
 *            fputs("Allocation error!\n", stderr);
 *            string_pool_no_duplicates_free(no_dups_pool);
 *            return 1;
 *        }
 *
 *        for (size_t j=0; j<=i; j++) {
 *            if (temp == allocated[j]) break;
 *            else if (allocated[j] == 0) {
 *                allocated[j] = temp;
 *                break;
 *            }
 *        }
 *    }
 *
 *    for (size_t i = 0; i < TEST_NUM_OF_PUTS; ++i) {
 *        if (allocated[i])
 *            printf("%s\n", allocated[i]);
 *    }
 *
 *    string_pool_no_duplicates_free(no_dups_pool);
 *    return 0;
 * }
 * ```
 *
 * The output of this test should display the only two strings declared in the beginning,
 * `string_0` and `string_1`, despite the amount of times we call ::string_pool_no_duplicates_put,
 * meaning that only those strings are really allocated.
 */
typedef struct string_pool_no_duplicates string_pool_no_duplicates_t;

/**
 * @brief   Creates a string pool with no duplicates.
 * @details The returned value is owned by the caller, and should be freed with
 *          ::string_pool_no_duplicates_free.
 *
 * @param block_capacity The number of characters of each block in a pool. See ::pool_create for
 *                       more information.
 *
 * @return The newly created pool, or `NULL` on allocation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
string_pool_no_duplicates_t *string_pool_no_duplicates_create(size_t block_capacity);

/**
 * @brief   Allocates space and copies a string to a string pool, if the string doesn't already
 *          exist in the pool.
 * @details That string does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::string_pool_no_duplicates_free. However, keep in mind that the pool must oulive the
 *          allocated string.
 *
 * @param pool Pool to allocate the string in, if necessary.
 * @param str  String to be copied to the pool, if necessary.
 *
 * @return The pointer to the string in @p pool, or `NULL` on allocation failure. It must be
 *         constant, as the same pointer may be returned when the same string is allocate.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
const char *string_pool_no_duplicates_put(string_pool_no_duplicates_t *pool, const char *str);

/**
 * @brief Frees memory allocated by a string pool without duplicates.
 * @param pool Pool data to be freed.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
void string_pool_no_duplicates_free(string_pool_no_duplicates_t *pool);

#endif
