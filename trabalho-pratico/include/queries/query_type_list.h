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
 * @file  query_type_list.h
 * @brief A data structure for a list of all supported queries.
 *
 * @anchor query_type_list_examples
 * ### Examples
 *
 * A `query_type_list_t` can be created the following way:
 *
 * ```c
 * query_type_list_t *query_type_list = query_type_list_create();
 * if (!query_type_list) {
 *     fprintf(stderr, "Allocation error of query definitions!\n");
 *     exit(1);
 * }
 * ```
 *
 * You usually only need to create one of these, that will be passed around the whole program.
 *
 * Suppose you want to perform actions related to the following query: `"1 EdSousa"`. From this
 * list, you can get the methods that define this query type, `1`:
 *
 * ```c
 * const query_type_t *type = query_type_list_get_by_index(query_type_list, 1);
 * ```
 *
 * If your query were `"200 EdSousa"`, because there aren't 200 queries,
 * `query_type_list_get_by_index(query_type_list, 200)` would return `NULL`.
 *
 * In the end, don't forget to call `query_type_list_free(query_type_list)`.
 */

#ifndef QUERY_TYPE_LIST_H
#define QUERY_TYPE_LIST_H

#include "queries/query_type.h"

/** @brief Number of queries supported (1 to ::QUERY_TYPE_LIST_COUNT). */
#define QUERY_TYPE_LIST_COUNT 10

/** @brief A list of all supported queries. */
typedef struct query_type_list query_type_list_t;

/**
 * @brief   Initializes a list of all supported queries.
 * @details This must be called in the beginning of the program, as most functions that deal with
 *          queries require a list of query definitions.
 * @return  A pointer to a `query_type_list_t` (that must be freed with ::query_type_list_free), or
 *          `NULL` on failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_list_examples).
 */
query_type_list_t *query_type_list_create(void);

/**
 * @brief   Gets a query definition by its numerical identifier (type).
 * @details Query indexing starts at `1` instead of `0`.
 *
 * @param query_type_list List of query definitions.
 * @param index           ::query_instance::type.
 *
 * @return A pointer to a ::query_type_t on success, or `NULL` if the index is out-of-bounds.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_list_examples).
 */
const query_type_t *query_type_list_get_by_index(query_type_list_t *query_type_list, size_t index);

/**
 * @brief Frees memory allocated by ::query_type_list_create.
 * @param query_type_list Value returned by ::query_type_list_create.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_list_examples).
 */
void query_type_list_free(query_type_list_t *query_type_list);

#endif
