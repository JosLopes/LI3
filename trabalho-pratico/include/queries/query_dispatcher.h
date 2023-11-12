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
 * @file  query_dispatcher.h
 * @brief Module responsible for the execution of queries.
 *
 * @anchor query_dispatcher_examples
 * ### Examples
 * TODO
 */

#ifndef QUERY_DISPATCHER_H
#define QUERY_DISPATCHER_H

#include <stdio.h>

#include "database/database.h"
#include "queries/query_instance_list.h"

/**
 * @brief Runs a single query.
 *
 * @param database        Database, so that the query can get information.
 * @param query_instance  Query to be run.
 * @param query_type_list List of known queries.
 * @param output          Where the query's result should be written to.
 */
void query_dispatcher_dispatch_single(database_t        *database,
                                      query_instance_t  *query_instance,
                                      query_type_list_t *query_type_list,
                                      FILE              *output);

/**
 * @brief Runs a list of queries.
 *
 * @param database            Database, so that the query can get information.
 * @param query_instance_list List of queries to be run.
 * @param query_type_list     List of known queries.
 * @param outputs             Where the queries' results will be written to. These should be in
 *                            the same as the @p query_instance_list after being sorted.
 */
void query_dispatcher_dispatch_list(database_t            *database,
                                    query_instance_list_t *query_instance_list,
                                    query_type_list_t     *query_type_list,
                                    FILE                 **outputs);

#endif
