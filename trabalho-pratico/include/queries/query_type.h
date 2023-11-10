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
 * @file  query_type.h
 * @brief A way to define a query based on its behavior.
 */

#ifndef QUERY_TYPE_H
#define QUERY_TYPE_H

#include <stddef.h>

/** @brief A definition of a query. */
typedef struct query_type query_type_t;

/**
 * @brief Type of the method called for parsing query arguments.
 *
 * @param argv Arguments of the query.
 * @param argc Number of query arguments.
 *
 * @return `NULL` on failure, other value on success.
 */
typedef void *(*query_type_parse_arguments_callback)(char **argv, size_t argc);

/**
 * @brief Type of the method called for freeing ::query_instance::argument_data.
 * @param argument_data ::query_instance::argument_data.
 */
typedef void (*query_type_free_query_instance_argument_data_callback)(void *argument_data);

/**
 * @brief Creates a query type, defining its behavior.
 *
 * @param parse_arguments                            Method that parses query arguments and
 *                                                   generates ::query_instance::argument_data.
 * @param free_query_instance_argument_data_callback Method called for freeing
 *                                                   ::query_instance::argument_data.
 *
 * @return A pointer to a new `malloc`-allocated ::query_type_t (or `NULL` on allocation failure).
 *         After being used, it must be `free`'d with ::query_type_free.
 */
query_type_t *query_type_create(
    query_type_parse_arguments_callback                   parse_arguments,
    query_type_free_query_instance_argument_data_callback free_query_instance_argument_data);

query_type_parse_arguments_callback query_type_get_parse_arguments_callback(query_type_t *type);

query_type_free_query_instance_argument_data_callback
    query_type_get_free_query_instance_argument_data_callback(query_type_t *type);

/**
 * @brief Frees memory in a ::query_type_t.
 * @param query Query to be deleted.
 */
void query_type_free(query_type_t *query);

#endif
