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
 * A query instance usually originates from a [query parser](@ref queryparser.h). You can also
 * create it using ::query_instance_create, followed by calling each one of the following setters:
 *
 * - ::query_instance_set_type;
 * - ::query_instance_set_formatted;
 * - ::query_instance_set_number_in_file (use 1 if not a query in a file);
 * - ::query_instance_set_argument_data (see ::query_type_parse_arguments_callback for your query
 *   type).
 *
 * To run the query you created, see ::query_dispatcher_dispatch_single.
 *
 * In the end, don't forget to call ::query_instance_free. ::query_instance_pooled_free only
 * applies if you haven't created your query instance with ::query_instance_create, but allocated
 * it in a pool / array.
 */

#ifndef QUERY_INSTANCE_H
#define QUERY_INSTANCE_H

#include <stdlib.h>
#include <string.h>

#include "queries/query_instance_typedef.h"
#include "queries/query_type_list.h"

/**
 * @brief Create a new query instance.
 * @return A new ::query_instance_t, that should be `free`'d with ::query_instance_free, or `NULL`
 *         on failure.
 */
query_instance_t *query_instance_create(void);

/**
 * @brief Sets the type of a query.
 * @param query Query instance to have its type set.
 * @param type  Type of the query.
 */
void query_instance_set_type(query_instance_t *query, size_t type);

/**
 * @brief Sets whether a query's output should be formatted or not.
 * @param query     Query instance to have its formatting flag set.
 * @param formatted If the query's output should be formatted or not.
 */
void query_instance_set_formatted(query_instance_t *query, int formatted);

/**
 * @brief Sets the number of the line a query was on.
 * @param query          Query instance to have its line number in the file set.
 * @param number_in_file The number of the line @p query was on (in the query file).
 */
void query_instance_set_number_in_file(query_instance_t *query, size_t number_in_file);

/**
 * @brief Adds data relating to argument parsing results to a query. Its data type will depend on
 *        the query's type.
 *
 * @param query         Query instance to have its formatting flag set.
 * @param argument_data Data resulting from parsing the query's arguments.
 */
void query_instance_set_argument_data(query_instance_t *query, void *argument_data);

/**
 * @brief  Gets the type of a query.
 * @param  query Query to get the type from.
 * @return The type of @p query.
 */
size_t query_instance_get_type(const query_instance_t *query);

/**
 * @brief  Gets whether a query's output should or not be formatted.
 * @param  query Query to get the formatted flag from.
 * @return Whether @p query 's output should or not be formatted.
 */
int query_instance_get_formatted(const query_instance_t *query);

/**
 * @brief  Gets the number of the line a query was on.
 * @param  query Query to get the number in file from.
 * @return The number of the line @p query was on.
 */
size_t query_instance_get_number_in_file(const query_instance_t *query);

/**
 * @brief  Gets data resulting from parsing the query's arguments.
 * @param  query Query to get argument data from.
 * @return Data resulting from parsing the query's arguments. Its data type will depend on the
 *         query's type.
 */
void *query_instance_get_argument_data(const query_instance_t *query);

/**
 * @brief   Gets the size of a ::query_instance_t in memory.
 * @details Useful for pool and contiguous array allocation.
 * @return  `sizeof(query_instance_t)`.
 */
size_t query_instance_sizeof(void);

/**
 * @brief   Frees memory used by a query instance, when it's stored in a pool.
 * @details Frees the contents of a `query_instance_t *`, but does not attempt to free the pointer
 *          itself.
 *
 * @param query           Query instance to be freed.
 * @param query_type_list List of supported queries (to know how to free `argument_data` in @p
 *                        query).
 */
void query_instance_pooled_free(query_instance_t *query, query_type_list_t *query_type_list);

/**
 * @brief Frees memory used by a query instance, created by ::query_instance_create.
 *
 * @param query           Query instance to be freed.
 * @param query_type_list List of supported queries (to know how to free `argument_data` in @p
 *                        query).
 */
void query_instance_free(query_instance_t *query, query_type_list_t *query_type_list);

#endif
