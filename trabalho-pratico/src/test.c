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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <stdio.h>

#include "utils/pool.h"

/** @brief Number of items in a pool block */
#define TEST_POOL_BLOCK_SIZE 1000

/** @brief Number of pool items to be allocated */
#define TEST_NUM_ITEMS 100000

/**
 * @brief The entry point to the test program.
 * @details Tests for pools.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    pool_t *pool = pool_create(int, TEST_POOL_BLOCK_SIZE);

    int *allocated[TEST_NUM_ITEMS] = {0};
    for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
        allocated[i] = pool_put_item(int, pool, &i);
        if (!allocated[i]) {
            fputs("Allocation error!\n", stderr);
            pool_free(pool);
            return 1;
        }
    }

    for (size_t i = 0; i < TEST_NUM_ITEMS; ++i) {
        printf("%d\n", *allocated[i]);
    }

    pool_free(pool);
    return 0;
}
