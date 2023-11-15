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
 * @file  query_type_list.c
 * @brief Implementation of methods in include/queries/query_type_list.h
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_list_examples).
 */

#include <stddef.h>
#include <stdlib.h>

#include "queries/q01.h"
#include "queries/qplaceholder.h"
#include "queries/query_type_list.h"

/** @brief Number of queries supported (1 to ::QUERY_TYPE_LIST_COUNT). */
#define QUERY_TYPE_LIST_COUNT 10

/**
 * @struct query_type_list
 * @brief Container structure for the list of all supported queries.
 *
 * @var query_type_list::list
 *     @brief List of all supported queries.
 */
struct query_type_list {
    query_type_t *list[QUERY_TYPE_LIST_COUNT];
};

query_type_list_t *query_type_list_create(void) {
    query_type_list_t *list = malloc(sizeof(struct query_type_list));
    if (!list)
        return NULL;

    query_type_t *(*constructors[QUERY_TYPE_LIST_COUNT])(void) = {q01_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create,
                                                                  qplaceholder_create};

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        list->list[i] = constructors[i]();

        if (!list->list[i]) { /* Allocation failure */
            for (size_t j = 0; j < i; ++j)
                query_type_free(list->list[j]);

            free(list);
            return NULL;
        }
    }

    return list;
}

query_type_t *query_type_list_get_by_index(query_type_list_t *query_type_list, size_t index) {
    if (1 <= index && index <= QUERY_TYPE_LIST_COUNT)
        return query_type_list->list[index - 1];
    return NULL;
}

void query_type_list_free(query_type_list_t *query_type_list) {
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        query_type_free(query_type_list->list[i]);
    free(query_type_list);
}
