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
 * @file    query_type_list.h
 * @brief   The list of all supported queries.
 * @details Essentially, a list of `vtable`s.
 *
 * @anchor query_type_list_example
 * ### Examples
 *
 * Suppose you want to perform actions related to the following query: `"1 EdSousa"`. From this
 * list, you can get the methods that define this query type, `1`:
 *
 * ```c
 * const query_type_t *type = query_type_list_get_by_index(1);
 * ```
 *
 * If your query were `"200 EdSousa"`, because there aren't 200 queries,
 * `query_type_list_get_by_index(200)` would return `NULL`.
 */

#ifndef QUERY_TYPE_LIST_H
#define QUERY_TYPE_LIST_H

#include <stddef.h>

#include "queries/query_type.h"

/** @brief Number of queries supported (1 to ::QUERY_TYPE_LIST_COUNT). */
#define QUERY_TYPE_LIST_COUNT 10

/**
 * @brief   Gets a query definition by its numerical identifier (type).
 * @details Query indexing starts at `1` instead of `0`.
 *
 * @param index Type of a query (::query_instance::type).
 *
 * @return A pointer to a ::query_type_t on success, or `NULL` if the index is out-of-bounds.
 *
 * #### Example
 * See [the header file's documentation](@ref query_type_list_example).
 */
const query_type_t *query_type_list_get_by_index(size_t index);

#endif
