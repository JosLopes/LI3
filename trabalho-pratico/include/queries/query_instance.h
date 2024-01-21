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
 * @file  query_instance.h
 * @brief An occurrence of a query (in a file, or inputted by the user).
 *
 * @anchor query_instance_examples
 * ### Examples
 *
 * A query instance usually originates from the [query parser](@ref query_parser.h). You can also
 * create it using ::query_instance_create, followed by calling each one of the following setters:
 *
 * - ::query_instance_set_type;
 * - ::query_instance_set_formatted;
 * - ::query_instance_set_line_in_file (use 1 if not a query in a file);
 * - ::query_instance_set_argument_data (using the ::query_type_parse_arguments_callback_t method
 *   for your query type).
 *
 * To run the query you created, see ::query_dispatcher_dispatch_single. In the end, don't forget to
 * call ::query_instance_free.
 */

#ifndef QUERY_INSTANCE_H
#define QUERY_INSTANCE_H

#include <stdlib.h>
#include <string.h>

/* clang-format off */
#ifndef query_instance_typedef
    /** @cond FALSE */
    #define query_instance_typedef
    /** @endcond */

    /** @brief An occurrence of a query (in a file, or inputted by the user). */
    typedef struct query_instance query_instance_t;
#endif
/* clang-format on */

#include "queries/query_type.h"

/**
 * @brief  Creates a new query instance.
 * @return A pointer to a new ::query_instance_t, that should be `free`d with ::query_instance_free,
 *         or `NULL` on failure.
 */
query_instance_t *query_instance_create(void);

/**
 * @brief  Creates a deep copy of a query instance.
 * @param  query Query to be copied.
 * @return A pointer to a copy of @p query, that must be `free`d with ::query_instance_free, `NULL`
 *         on allocation failure / or invalid query type.
 */
query_instance_t *query_instance_clone(const query_instance_t *query);

/**
 * @brief Sets the type of a query instance.
 * @param query Query instance to have its type set.
 * @param type  Type of the query instance.
 */
void query_instance_set_type(query_instance_t *query, const query_type_t *type);

/**
 * @brief Sets whether or not a query's output should be formatted.
 * @param query     Query instance to have its formatting flag set.
 * @param formatted Whether or not the query's output should be formatted.
 */
void query_instance_set_formatted(query_instance_t *query, int formatted);

/**
 * @brief Sets the number of the line a query instance was on.
 * @param query        Query instance to have its line number in the file set.
 * @param line_in_file The number of the line the query instance was on (in the query file). Leave
 *                     `1` for an interactive mode query.
 */
void query_instance_set_line_in_file(query_instance_t *query, size_t line_in_file);

/**
 * @brief   Adds parsed arguments to a query. Its data type will depend on the query's type.
 * @details For any query instance, ::query_instance_set_type must be called before this setter.
 *
 * @param query           Query instance to have its arguments set.
 * @param argument_data   Data resulting from parsing the query's arguments.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure or query type not set.
 */
int query_instance_set_argument_data(query_instance_t *query, const void *argument_data);

/**
 * @brief  Gets the type of a query instance.
 * @param  query Query instance to get the type from.
 * @return The type of @p query.
 */
const query_type_t *query_instance_get_type(const query_instance_t *query);

/**
 * @brief  Gets whether or not a query's output should be formatted.
 * @param  query Query instance to get the formatted flag from.
 * @return Whether or not @p query 's output should be formatted.
 */
int query_instance_get_formatted(const query_instance_t *query);

/**
 * @brief  Gets the number of the line a query instance was on.
 * @param  query Query instance to get the line number from.
 * @return The number of the line @p query was on.
 */
size_t query_instance_get_line_in_file(const query_instance_t *query);

/**
 * @brief  Gets a query instance's parsed arguments.
 * @param  query Query instance to get parsed arguments from.
 * @return Data resulting from parsing the query's arguments. Its data type will depend on the
 *         query's type.
 */
const void *query_instance_get_argument_data(const query_instance_t *query);

/**
 * @brief Frees memory used by a query instance.
 * @param query Query instance to be `free`d.
 */
void query_instance_free(query_instance_t *query);

#endif
