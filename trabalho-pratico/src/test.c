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
 * @brief Contains the entry point to test the program.
 */

#include <stdio.h>

#include "utils/single_pool_id_linked_list.h"

/**
 * @brief The entry point to the test program.
 * @details Tests for linked lists.

 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    /* Block size should be higher in practice */
    pool_t *ll_pool = single_pool_id_linked_list_create_pool(20);
    if (!ll_pool) {
        fputs("Allocation failure!\n", stderr);
        return 1;
    }

    single_pool_id_linked_list_t *ll = single_pool_id_linked_list_create();

    for (int i = 10; i >= 1; --i) {
        ll = single_pool_id_linked_list_append_beginning(ll_pool, ll, i);
        if (!ll) {
            fputs("Allocation failure!\n", stderr);
            return 1;
        }
    }

    single_pool_id_linked_list_t *iter_ll = ll;
    while (iter_ll != NULL) {
        printf("%" PRIu64 "\n", single_pool_id_linked_list_get_value(iter_ll));
        iter_ll = single_pool_if_linked_list_get_next(iter_ll);
    }

    pool_free(ll_pool);
    return 0;
}
