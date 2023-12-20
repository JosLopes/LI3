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
 * @file  single_pool_id_linked_list.c
 * @brief Implementation of methods in include/utils/single_pool_id_linked_list.h
 *
 * ### Examples
 * See [the header file's documentation](@ref single_pool_id_linked_list_examples).
 */

#include "utils/single_pool_id_linked_list.h"

/**
 * @struct single_pool_id_linked_list
 * @brief A node in a ::single_pool_id_linked_list_t.
 *
 * @var single_pool_id_linked_list::value
 *     @brief Value stored in this node.
 * @var single_pool_id_linked_list::next
 *     @brief Pointer to the next node.
 */
struct single_pool_id_linked_list {
    uint32_t                      value;
    single_pool_id_linked_list_t *next;
};

single_pool_id_linked_list_t *single_pool_id_linked_list_create(void) {
    return NULL;
}

single_pool_id_linked_list_t *
    single_pool_id_linked_list_append_beginning(pool_t                       *allocator,
                                                single_pool_id_linked_list_t *list,
                                                uint32_t                      value) {

    single_pool_id_linked_list_t item = {.value = value, .next = list};
    return pool_put_item(single_pool_id_linked_list_t, allocator, &item);
}

pool_t *single_pool_id_linked_list_create_pool(size_t block_capacity) {
    return pool_create(single_pool_id_linked_list_t, block_capacity);
}

uint32_t single_pool_id_linked_list_get_value(const single_pool_id_linked_list_t *list) {
    return list->value;
}

const single_pool_id_linked_list_t *
    single_pool_id_linked_list_get_next(const single_pool_id_linked_list_t *list) {

    return list->next;
}

size_t single_pool_id_linked_list_length(const single_pool_id_linked_list_t *list) {
    size_t length = 0;
    while (list) {
        length++;
        list = list->next;
    }
    return length;
}
