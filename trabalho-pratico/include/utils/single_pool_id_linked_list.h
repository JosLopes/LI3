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
 * @file    single_pool_id_linked_list.h
 * @brief   A linked lists that allows for multiple linked lists to allocate nodes in the same
 *          ::pool_t.
 * @details It's designed to store `uint32_t`s, i.e., flight and reservation IDs.
 *
 * @anchor single_pool_id_linked_list_examples
 * ### Example
 *
 * The following example shows how to create a linked list, add elements to it, and iterate over it.
 *
 * ```c
 * #include <stdio.h>
 * #include "utils/single_pool_id_linked_list.h"
 *
 * int main(void) {
 *     // Block size should be higher in practice
 *     pool_t *ll_pool = single_pool_id_linked_list_create_pool(20);
 *     if (!ll_pool) {
 *         fputs("Allocation failure!\n", stderr);
 *         return 1;
 *     }
 *
 *     single_pool_id_linked_list_t *ll = single_pool_id_linked_list_create();
 *
 *     for (int i = 10; i >= 1; --i) {
 *         ll = single_pool_id_linked_list_append_beginning(ll_pool, ll, i);
 *         if (!ll) {
 *             fputs("Allocation failure!\n", stderr);
 *             return 1;
 *         }
 *     }
 *
 *     single_pool_id_linked_list_t *iter_ll = ll;
 *     while (iter_ll != NULL) {
 *         printf("%" PRIu32 "\n", single_pool_id_linked_list_get_value(iter_ll));
 *         iter_ll = single_pool_if_linked_list_get_next(iter_ll);
 *     }
 *
 *     pool_free(ll_pool);
 *     return 0;
 * }
 * ```
 */

#ifndef SINGLE_POOL_ID_LINKED_LIST_H
#define SINGLE_POOL_ID_LINKED_LIST_H

#include <inttypes.h>
#include <stddef.h>

#include "utils/pool.h"

/**
 * @brief   A linked lists that allows for multiple linked lists to allocate nodes in the same
 *          ::pool_t.
 * @details It's designed to store `uint32_t`s, i.e., flight and reservation IDs.
 */
typedef struct single_pool_id_linked_list single_pool_id_linked_list_t;

/**
 * @brief   Creates a new ::single_pool_id_linked_list_t.
 * @details No need to `free` it, as this will be pool allocated.
 */
single_pool_id_linked_list_t *single_pool_id_linked_list_create(void);

/**
 * @brief   Creates a pool for use with ::single_pool_id_linked_list_t.
 * @details Do not forget to free the result using ::pool_free.
 *
 * @param block_capacity Number of nodes in each pool block. See pool.h for more information.
 *
 * @return A pointer to a ::pool_t on success, or `NULL` on failure.
 */
pool_t *single_pool_id_linked_list_create_pool(size_t block_capacity);

/**
 * @brief Appends an element to the beginning of a linked list.
 *
 * @param allocator Where to allocate the new node. Must have been creating using
 *                  ::single_pool_id_linked_list_create_pool.
 * @param list      List to add @p value to.
 * @param value     Value to be added to @p list.
 *
 * @return The new beginning of the linked list (or `NULL` on allocation failure);
 */
single_pool_id_linked_list_t *
    single_pool_id_linked_list_append_beginning(pool_t                       *allocator,
                                                single_pool_id_linked_list_t *list,
                                                uint32_t                      value);

/**
 * @brief  Gets the value of the first node in a ::single_pool_id_linked_list_t.
 * @param  list List to get the value of the first node from.
 * @return The value of the first node in @p list.
 */
uint32_t single_pool_id_linked_list_get_value(const single_pool_id_linked_list_t *list);

/**
 * @brief  Gets the next list from a ::single_pool_id_linked_list_t.
 * @param  list List to get the next list from.
 * @return The linked list succeding @p list. `NULL` means the end of the list has been found.
 */
const single_pool_id_linked_list_t *
    single_pool_id_linked_list_get_next(const single_pool_id_linked_list_t *list);

/**
 * @brief  Gets the length of a ::single_pool_id_linked_list_t.
 * @param  list List to get length from.
 * @return The length of the @p list.
 */
size_t single_pool_id_linked_list_length(const single_pool_id_linked_list_t *list);

#endif
