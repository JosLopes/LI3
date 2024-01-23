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
 * queries. Essentially, you need to [load a database from a dataset](@ref dataset_loader_examples)
 * and [parse a query file](@ref query_file_parser_examples). The hardest part is creating the set
 * of query writers for all queries. To make sure the output file order is correct, please use
 * ::query_instance_list_iter.
 */

#ifndef QUERY_DISPATCHER_H
#define QUERY_DISPATCHER_H

#include "database/database.h"
#include "queries/query_instance_list.h"
#include "testing/performance_metrics.h"

/**
 * @brief   Runs a single query.
 * @details If you want to run multiple queries, do not call this method multiple times, as that
 *          will be very innefficient. Queries generate statistical data, shared by all queries of
 *          the same type to improve performance. That can only be taken advantage of by using
 *          ::query_dispatcher_dispatch_list.
 *
 * @param database       Database, so that the query can get information.
 * @param query_instance Query to be run.
 * @param output         Where the query's result should be written to.
 *
 * @retval 0 Query preparation success. Running the query itself might have silently failed.
 * @retval 1 Allocation failure or invalid @p query_instance.
 */
int query_dispatcher_dispatch_single(const database_t       *database,
                                     const query_instance_t *query_instance,
                                     query_writer_t         *output);

/**
 * @brief Runs a list of queries.
 *
 * @param database            Database, so that the queries can get information.
 * @param query_instance_list List of queries to be run. Cannot be `const`, as this list may get
 *                            sorted. However, no new items will be added to it.
 * @param outputs             Where the queries' results will be written to. These should be in
 *                            the same order as @p query_instance_list after being sorted.
 * @param metrics             Where to write profiling data to. Can be `NULL` for no profiling.
 */
void query_dispatcher_dispatch_list(const database_t      *database,
                                    query_instance_list_t *query_instance_list,
                                    query_writer_t *const *outputs,
                                    performance_metrics_t *metrics);

#endif
