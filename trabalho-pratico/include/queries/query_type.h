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
 * @file    query_type.h
 * @brief   A way to define a query based on its behavior.
 * @details A query is defined by a set of callbacks, one for each specific function that can be
 *          performed (a way around C's lack of polymorphism).
 *
 * @anchor query_type_examples
 * ### Examples
 *
 * The following summary is for people who are interested in implementing their own queries. If
 * you just want to use existing queries, see ::query_type_list_t.
 *
 * Check out implementations of existing queries (q1.h to q10.h). In summary, here is what each
 * callback that needs to be defined does:
 *
 * - ::query_type_parse_arguments_callback_t parses arguments after the query type + formatting
 *   flag. For example, in `1f a b c`, this method is called with {"a", "b", "c"} for `argv` and `3`
 *   for `argc`. Note that the values in `argv` must be copied in case you want to store them for
 *   later, as they may change when parsing a query file. The function must return the value that
 *   will be stored in ::query_instance::argument_data (or `NULL` on failure).
 *
 * - ::query_type_free_query_instance_argument_data_callback_t frees data generated by
 *   ::query_type_parse_arguments_callback_t.
 *
 * - ::query_type_generate_statistics_callback_t generates statistical data to be used by all
 *   queries of the same type.
 *
 * - ::query_type_free_statistics_callback_t frees data generated by
 *   ::query_type_generate_statistics_callback.
 *
 * - ::query_type_execute_callback_t executes a query.
 *
 * After defining these methods, create a constructor for your query using ::query_type_create.
 * Remember that any ::query_type_create call must have a ::query_type_free match. This is usually
 * automatically handled by ::query_type_list_t. If you're creating a new query, you must modify
 * the source code of ::query_type_list_create to take your new query into account.
 *
 * For accessing methods of existing queries, use the getters defined in this module.
 */

#ifndef QUERY_TYPE_H
#define QUERY_TYPE_H

#include <stddef.h>
#include <stdio.h>

#include "database/database.h"
#include "queries/query_instance_typedef.h"
#include "queries/query_writer.h"

/** @brief A definition of a query. */
typedef struct query_type query_type_t;

/**
 * @brief Type of the method called for parsing query arguments.
 *
 * @param argv Arguments of the query. Do not store these pointers without first making a copy of
 *             each string. They are non-constant and will be quickily discarded, so you can modify
 *             them during your parsing.
 * @param argc Number of query arguments.
 *
 * @return `NULL` on failure, other value on success. This value will be stored in
 *         ::query_instance::argument_data.
 */
typedef void *(*query_type_parse_arguments_callback_t)(char *const *argv, size_t argc);

/**
 * @brief  Type of the method called for cloning query arguments.
 * @param  args_data Value returned by ::query_type_parse_arguments_callback_t.
 * @return A deep clone of @p args, `NULL` on allocation failure.
 */
typedef void *(*query_type_clone_arguments_callback_t)(const void *args_data);

/**
 * @brief Type of the method called for freeing ::query_instance::argument_data.
 * @param argument_data ::query_instance::argument_data.
 */
typedef void (*query_type_free_query_instance_argument_data_callback_t)(void *argument_data);

/**
 * @brief   Type of the method called to generate statistical data before running all queries of the
 *          same type.
 * @details Can be `NULL`, not to generate any statistics.
 *
 * @param database  Database to collect statistics
 * @param instances List of query instances that will need to be processed.
 * @param n         Number of query instances in @p instances.
 *
 * @return `NULL` on failure, another value on success. If you do not plan on collecting statistical
 *         data, this callback itself can be left as `NULL`. Statistical data will be global to
 *         all queries of the same type.
 */
typedef void *(*query_type_generate_statistics_callback_t)(const database_t       *database,
                                                           const query_instance_t *instances,
                                                           size_t                  n);

/**
 * @brief   Frees data generated by ::query_type_generate_statistics_callback.
 * @details Can be `NULL`, only if the query type's ::query_type_generate_statistics_callback is
 *          also `NULL`.
 * @param   statistics Data generated by ::query_type_generate_statistics_callback.
 */
typedef void (*query_type_free_statistics_callback_t)(void *statistics);

/**
 * @brief Executes a query.
 *
 * @param database   Database to perform data lookups.
 * @param statistics Data generated by ::query_type_generate_statistics_callback, or `NULL` if that
 *                   callback isn't defined.
 * @param instance   Query instance to execute.
 * @param output     Where to write query results to.
 *
 * @return `0` on success, other value on failure.
 */
typedef int (*query_type_execute_callback_t)(const database_t       *database,
                                             const void             *statistics,
                                             const query_instance_t *instance,
                                             query_writer_t         *output);

/**
 * @brief   Creates a query type, defining its behavior.
 * @details For parameter description, see the type name of each parameter.
 *
 * @return A pointer to a new `malloc`-allocated ::query_type_t (or `NULL` on allocation failure).
 *         After being used, it must be `free`'d with ::query_type_free.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_examples).
 */
query_type_t *query_type_create(
    query_type_parse_arguments_callback_t                   parse_arguments,
    query_type_clone_arguments_callback_t                   clone_arguments,
    query_type_free_query_instance_argument_data_callback_t free_query_instance_argument_data,
    query_type_generate_statistics_callback_t               generate_statistics,
    query_type_free_statistics_callback_t                   free_statistics,
    query_type_execute_callback_t                           execute);

/**
 * @brief  Creates a deep copy of a query type.
 * @param  type Query type to be copied.
 * @return A pointer to a copy of @p type, or `NULL` on allocation failure.
 */
query_type_t *query_type_clone(const query_type_t *type);

/**
 * @brief  Gets the method called for parsing query arguments from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for parsing query arguments from.
 * @return The method called for parsing query arguments.
 */
query_type_parse_arguments_callback_t
    query_type_get_parse_arguments_callback(const query_type_t *type);

/**
 * @brief  Gets the method called for cloning query arguments from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for cloning query arguments from.
 * @return The method called for deep-cloning query arguments.
 */
query_type_clone_arguments_callback_t
    query_type_get_clone_arguments_callback(const query_type_t *type);

/**
 * @brief  Gets the method called for freeing ::query_instance::argument_data from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for freeing ::query_instance::argument_data
 *         from.
 * @return The method called for freeing ::query_instance::argument_data.
 */
query_type_free_query_instance_argument_data_callback_t
    query_type_get_free_query_instance_argument_data_callback(const query_type_t *type);

/**
 * @brief  Gets the method called for generating statistical data from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for generating statistical data from.
 * @return The method called for generating statistical data.
 */
query_type_generate_statistics_callback_t
    query_type_get_generate_statistics_callback(const query_type_t *type);

/**
 * @brief  Gets the method called for freeing statistical data from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for freeing statistical data from.
 * @return The method called for freeing statistical data.
 */
query_type_free_statistics_callback_t
    query_type_get_free_statistics_callback(const query_type_t *type);

/**
 * @brief  Gets the method called for executing a query from a ::query_type_t.
 * @param  type ::query_type_t to get the method called for query execution from.
 * @return The method called for executing a query.
 */
query_type_execute_callback_t query_type_get_execute_callback(const query_type_t *type);

/**
 * @brief Frees memory in a ::query_type_t.
 * @param query Query to be deleted.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_type_examples).
 */
void query_type_free(query_type_t *query);

#endif
