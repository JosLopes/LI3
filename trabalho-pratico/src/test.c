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
 * @file test.c
 * @brief Contains the entry point to the test program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils/string_pool_no_duplicates.h"

/* In practice, this number should be way larger */
#define TEST_POOL_BLOCK_SIZE 16

#define TEST_NUM_OF_PUTS 10

/**
 * @brief The entry point to the test program.
 * @details Tests for query parsing.
 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    string_pool_no_duplicates_t *no_dups_pool =
        string_pool_no_duplicates_create(TEST_POOL_BLOCK_SIZE);

    time_t t;
    srand((unsigned) time(&t));

    const char *string_0 = "Very creative string!";
    const char *string_1 = "Hello, world!";

    char *allocated[TEST_NUM_OF_PUTS] = {0};
    for (size_t i = 0; i < TEST_NUM_OF_PUTS; ++i) {
        int         r    = rand() % 2 == 1; /*Choose between string or string_1*/
        const char *temp = string_pool_no_duplicates_put(no_dups_pool, r ? string_0 : string_1);
        if (!temp) {
            fputs("Allocation error!\n", stderr);
            string_pool_no_duplicates_free(no_dups_pool);
            return 1;
        }

        for (size_t j = 0; j <= i; j++) {
            if (temp == allocated[j])
                break;
            else if (allocated[j] == 0) {
                /*For test purposes, we can type cast temp as a `char *`*/
                allocated[j] = (char *) temp;
                break;
            }
        }
    }

    for (size_t i = 0; i < TEST_NUM_OF_PUTS; ++i) {
        if (allocated[i])
            printf("%s\n", allocated[i]);
    }

    string_pool_no_duplicates_free(no_dups_pool);
    return 0;
}
