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

#include "utils/string_pool.h"

/**
 * @file    string_pool_no_duplicates.h
 * @brief   An allocator without duplicates for strings.
 * @details Uses the same methods applied to normal string pools, with the addition of a
 *          verification that assures that there are no duplicated strings inside this type of
 *          pools. This improves memory management by avoiding storing identical pieces of data in
 *          separate memory locations.
 *
 * @anchor string_pool_no_duplicates_examples
 * ### Examples
 *
 * The following example allocates multiple strings in a string pool without duplicates, and every
 * new pointer thats returned from the allocator is stored in an array. At the end, the program
 * should only print `n` strings,`n` being the quantity of different strings, all different from one
 * another.
 *
 * ```c
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include <time.h>
 * #include "utils/string_pool_no_duplicates.h"

 * //In practice, this number should be way larger.
 * #define TEST_POOL_BLOCK_SIZE 16
 *
 * #define TEST_NUM_OF_PUTS 10
 *
 * int main(void) {
 *    string_pool_no_duplicates_t *no_dups_pool =
 *        string_pool_no_duplicates_create(TEST_POOL_BLOCK_SIZE);
 *
 *    time_t t;
 *    srand((unsigned) time(&t));
 *
 *    const char *string_0 = "Very creative string!";
 *    const char *string_1 = "Hello, world!";
 *
 *    char *allocated[TEST_NUM_OF_PUTS] = {0};
 *    for (size_t i = 0; i < TEST_NUM_OF_PUTS; ++i) {
 *        int r = rand() % 2 == 1; //Choose between string or string_1.
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
 *                //For test purposes, we can type cast temp as a `char *`.
 *                allocated[j] = (char *) temp;
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
 *}
 *```
 */
typedef struct string_pool_no_duplicates string_pool_no_duplicates_t;

/**
 * @brief   Creates a string pool with no duplicates.
 * @details The returned value is owned by the caller, and should be freed with
 *          ::string_pool_no_duplicates_free.
 *
 * @param block_capacity The number of characters of each block in a pool.
 *
 * @return An allocated pool, or `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
string_pool_no_duplicates_t *string_pool_no_duplicates_create(size_t block_capacity);

/**
 * @brief Allocates space and copies a string to a string pool, if the string doesn't already exist
 *        in the pool.
 * @details That string does not need to be `free`'d, as that's done when @p pool itself is freed in
 *          ::string_pool_no_duplicates_free. If an equal string already exists, the function only
 *          returns a `const` pointer to it.
 *
 * @param pool_data Stores the pool to allocate the string in, if needed.
 * @param str       String to be copied to the pool, if needed.
 *
 * @return The pointer an already existing string if @p str equals other string already in the pool;
 *         the pointer to a newly allocated string if @p str is new to the pull, or `NULL` on
 *         failure.
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
const char *string_pool_no_duplicates_put(string_pool_no_duplicates_t *pool_data, const char *str);

/**
 * @brief  Gets the pool of strings from a pool without duplicates.
 * @param  pool_data Pool data to get the string pool from.
 * @return The @p pool_data string pool.
 */
string_pool_t *string_pool_no_duplicates_get_strings(string_pool_no_duplicates_t *pool_data);

/**
 * @brief Frees memory allocated by a string pool without duplicates.
 * @param pool_data Pool data to be freed.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_pool_no_duplicates_examples).
 */
void string_pool_no_duplicates_free(string_pool_no_duplicates_t *pool_data);

#endif
