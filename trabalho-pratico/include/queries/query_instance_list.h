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
 * @file    query_instance_list.h
 * @brief   A list of ::query_instance_t ordered by query type.
 * @details This could be, for example, a list of queries coming from a file.
 *
 * @anchor query_instance_list_examples
 * ### Examples
 * TODO - query instance list examples (when the rest of the query system, including dispatcher, is
 *        ready)
 */

#ifndef QUERY_INSTANCE_LIST_H
#define QUERY_INSTANCE_LIST_H

#include "queries/query_instance.h"

/** @brief A list of ::query_instance_t. */
typedef struct query_instance_list query_instance_list_t;

/**
 * @brief Method called for every set of query instances of the same type, in
 *        ::query_instance_list_iter_types.
 *
 * @param user_data Pointer, kept from call to call, so that this callback can modify the program's
 *                  state.
 * @param instances All query instances of the same type.
 * @param n         Number of instances in @p instances. It's guaranteed that there'll be no empty
 *                  sets.
 *
 * @return `0` on success, other value for immediate termination of iteration.
 */
typedef int (*query_instance_list_iter_types_callback)(void             *user_data,
                                                       query_instance_t *instances,
                                                       size_t            n);

/**
 * @brief Method called for every query in a query list, used by ::query_instance_list_iter.
 *
 * @param user_data Pointer, kept from call to call, so that this callback can modify the program's
 *                  state.
 * @param instance  Query instance.
 *
 * @return `0` on success, other value for immediate termination of iteration.
 */
typedef int (*query_instance_list_iter_callback)(void *user_data, query_instance_t *instance);

/**
 * @brief   Creates an empty list of ::query_instance_t.
 * @details This value must be `free`'d with ::query_instance_list_free.
 * @return  A new ::query_instance_list_t, or `NULL` on failure.
 */
query_instance_list_t *query_instance_list_create(void);

/**
 * @brief   **Moves** a query instance into a list of query instances.
 * @details Because this operation is a move, when ::query_instance_list_free gets called,
 *          ::query_instance_pooled_free will be called for every @p query you provide to this
 *          method. See ::query_instance_list_free_no_internals to avoid those deletions.
 *
 * @param list  List of query instances to add @p query to.
 * @param query Query instance to be added to @p list.
 */
void query_instance_list_add(query_instance_list_t *list, query_instance_t *query);

/**
 * @brief Iterates over every set of queries of each type.
 *
 * @param list      List of query instances.
 * @param callback  Callback called for every set of queries of each type.
 * @param user_data Value passed to @p callback, so that it can modify the program's state.
 *
 * @return The last value returned by @p callback (will always be `0` on success).
 */
int query_instance_list_iter_types(query_instance_list_t                  *list,
                                   query_instance_list_iter_types_callback callback,
                                   void                                   *user_data);

/**
 * @brief Iterates over every query in a query instance lst.
 *
 * @param list      List of query instances.
 * @param callback  Callback called for every set of queries of each type.
 * @param user_data Value passed to @p callback, so that it can modify the program's state.
 *
 * @return The last value returned by @p callback (will always be `0` on success).
 */
int query_instance_list_iter(query_instance_list_t            *list,
                             query_instance_list_iter_callback callback,
                             void                             *user_data);

/**
 * @brief  Gets the length of a ::query_instance_list_t.
 * @param  list List to get the length from.
 * @return The length of @p list.
 */
size_t query_instance_list_get_length(query_instance_list_t *list);

/**
 * @brief Frees memory allocated by ::query_instance_list_create.
 *
 * @param list List allocated by ::query_instance_list_create.
 * @param query_type_list List of supported queries (to know how to free `argument_data` in each
 *                        query instance in the list).
 */
void query_instance_list_free(query_instance_list_t *list, query_type_list_t *query_type_list);

/**
 * @brief Frees memory allocated by ::query_instance_list_create, but doesn't free the query
 *        instances inside it.
 *
 * @param list List allocated by ::query_instance_list_create.
 */
void query_instance_list_free_no_internals(query_instance_list_t *list);

#endif
