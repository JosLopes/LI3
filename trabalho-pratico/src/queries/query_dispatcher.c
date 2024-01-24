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
 * @file  query_dispatcher.c
 * @brief Implementation of methods in include/queries/query_dispatcher.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_dispatcher_examples).
 */

#include <stddef.h>

#include "queries/query_dispatcher.h"

int query_dispatcher_dispatch_single(const database_t       *database,
                                     const query_instance_t *query_instance,
                                     query_writer_t         *output) {

    query_instance_list_t *const list = query_instance_list_create();
    if (!list)
        return 1;

    if (query_instance_list_add(list, query_instance)) {
        query_instance_list_free(list);
        return 1;
    }

    query_dispatcher_dispatch_list(database, list, &output, NULL);
    query_instance_list_free(list);
    return 0;
}

/**
 * @struct query_dispatcher_data_t
 * @brief  Data needed while dispatching a list of queries.
 *
 * @var query_dispatcher_data_t::database
 *     @brief Database, so that queries can access data.
 * @var query_dispatcher_data_t::outputs
 *     @brief Where to output query results to.
 * @var query_dispatcher_data_t::i
 *     @brief Number of queries processed until now.
 * @var query_dispatcher_data_t::metrics
 *     @brief Performance metrics where to write profiling information to.
 */
typedef struct {
    const database_t *const      database;
    query_writer_t *const *const outputs;
    size_t                       i;
    performance_metrics_t *const metrics;
} query_dispatcher_data_t;

/**
 * @brief Gets called for each set of queries of the same type, to execute them.
 *
 * @param user_data A pointer to a ::query_dispatcher_data_t.
 * @param n         Number of queries to be processed.
 * @param instances Queries of the same type to be processed.
 *
 * @return Always `0`, even on failure. No query execution failures shall halt the program.
 */
int __query_dispatcher_query_set_callback(void                         *user_data,
                                          size_t                        n,
                                          const query_instance_t *const instances[n]) {
    query_dispatcher_data_t *const dispatcher_data = user_data;

    const query_type_t *const type     = query_instance_get_type(instances[0]);
    const size_t              type_num = query_type_get_type_number(type);

    const query_type_generate_statistics_callback_t generate_stats =
        query_type_get_generate_statistics_callback(type);
    const query_type_free_statistics_callback_t free_stats =
        query_type_get_free_statistics_callback(type);
    const query_type_execute_callback_t execute = query_type_get_execute_callback(type);

    void *statistics = NULL;
    if (generate_stats) {
        performance_metrics_start_measuring_query_statistics(dispatcher_data->metrics, type_num);
        statistics = generate_stats(dispatcher_data->database, n, instances);
        performance_metrics_stop_measuring_query_statistics(dispatcher_data->metrics, type_num);

        if (!statistics)
            return 0; /* Query statistical failure */
    }

    for (size_t j = 0; j < n; ++j) {
        const size_t line = query_instance_get_line_in_file(instances[j]);

        performance_metrics_start_measuring_query_execution(dispatcher_data->metrics,
                                                            type_num,
                                                            line);
        execute(dispatcher_data->database,
                statistics,
                instances[j],
                dispatcher_data->outputs[dispatcher_data->i + j]); /* Ignore returned result */
        performance_metrics_stop_measuring_query_execution(dispatcher_data->metrics,
                                                           type_num,
                                                           line);
    }
    dispatcher_data->i += n;

    if (free_stats)
        free_stats(statistics);
    return 0;
}

void query_dispatcher_dispatch_list(const database_t      *database,
                                    query_instance_list_t *query_instance_list,
                                    query_writer_t *const *outputs,
                                    performance_metrics_t *metrics) {

    query_dispatcher_data_t dispatcher_data = {.database = database,
                                               .outputs  = outputs,
                                               .i        = 0,
                                               .metrics  = metrics};
    query_instance_list_iter_types(query_instance_list,
                                   __query_dispatcher_query_set_callback,
                                   &dispatcher_data);
}
