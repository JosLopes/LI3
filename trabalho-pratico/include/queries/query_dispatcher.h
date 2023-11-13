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
 *
 * batch_mode.c is a great example of everything you need to do to be able to start dispatching
 * queries. Essentially, you need to [load a database from a dataset](@ref dataset_loader_examples),
 * [create a list of query definitions](@ref query_type_list_examples) and
 * [load a query file](@ref query_file_parser_examples). The hardest part is creating the set of
 * output files for all queries. To make sure the output file order is correct,
 * ::query_instance_list_iter is used.
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
 * @param database            Database, so that the queries can get information.
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
