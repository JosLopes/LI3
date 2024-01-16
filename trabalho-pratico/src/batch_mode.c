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
 * @file  batch_mode.c
 * @brief Implementation of methods in batch_mode.h
 *
 * ### Examples
 * See [the header file's documentation](@ref batch_mode_examples).
 */

#include <limits.h>
#include <stdio.h>

#include "dataset/dataset_loader.h"
#include "queries/query_dispatcher.h"
#include "queries/query_file_parser.h"

/**
 * @struct batch_mode_iter_data_t
 * @brief  Data structure used for query iteration in ::__batch_mode_init_file_callback.
 *
 * @var batch_mode_iter_data_t::outputs
 *     @brief Where to write opened query output writers to.
 * @var batch_mode_iter_data_t::i
 *     @brief Index of the query being currently dealt with.
 */
typedef struct {
    query_writer_t **const outputs;
    size_t                 i;
} batch_mode_iter_data_t;

/**
 * @brief Called for each query, to create the writer to which the query output will be written to.
 *
 * @param user_data A pointer to a ::batch_mode_iter_data.
 * @param instance  Query query instance, whose output should be outputted.
 */
int __batch_mode_init_file_callback(void *user_data, const query_instance_t *instance) {
    batch_mode_iter_data_t *const iter_data = user_data;

    /* Parent directory creation is assured by error file output while loading the dataset */
    char path[PATH_MAX];
    sprintf(path, "Resultados/command%zu_output.txt", query_instance_get_number_in_file(instance));

    iter_data->outputs[iter_data->i] =
        query_writer_create(path, query_instance_get_formatted(instance));

    if (!iter_data->outputs[iter_data->i]) {
        /* On failure, delete all writers already created */
        for (size_t j = 0; j < iter_data->i; ++j)
            query_writer_free(iter_data->outputs[j]);
        return 1;
    }

    iter_data->i++;
    return 0;
}

int batch_mode_run(const char            *dataset_dir,
                   const char            *query_file_path,
                   performance_metrics_t *metrics) {

    int retval = 0;

    query_type_list_t *const query_type_list = query_type_list_create();
    if (!query_type_list) {
        retval = 1;
        fputs("Failed to allocate query definitions!\n", stderr);
        goto DEFER_0;
    }

    FILE *const query_file = fopen(query_file_path, "r");
    if (!query_file) {
        retval = 1;
        fputs("Failed to read query file!\n", stderr);
        goto DEFER_1;
    }

    query_instance_list_t *const query_instance_list =
        query_file_parser_parse(query_file, query_type_list);
    if (!query_instance_list) {
        retval = 1;
        fputs("Failed to allocate list of queries!\n", stderr);
        goto DEFER_2;
    }

    database_t *const database = database_create();
    if (!database) {
        retval = 1;
        fputs("Failed to allocate database!\n", stderr);
        goto DEFER_3;
    }

    if (dataset_loader_load(database, dataset_dir, "Resultados", metrics)) {
        retval = 1;
        fputs("Failed to load dataset files!\n", stderr);
        goto DEFER_4;
    }

    query_writer_t **const query_outputs =
        malloc(sizeof(query_writer_t *) * query_instance_list_get_length(query_instance_list));
    if (!query_outputs) {
        retval = 1;
        fputs("Failed to allocate list of query outputs!\n", stderr);
        goto DEFER_4;
    }

    batch_mode_iter_data_t iter_data = {.outputs = query_outputs, .i = 0};
    if (query_instance_list_iter(query_instance_list,
                                 __batch_mode_init_file_callback,
                                 &iter_data)) {
        retval = 1;
        fputs("Failed to open one of the query outputs!\n", stderr);
        goto DEFER_5;
    }

    query_dispatcher_dispatch_list(database,
                                   query_instance_list,
                                   query_type_list,
                                   query_outputs,
                                   metrics);

    for (size_t i = 0; i < query_instance_list_get_length(query_instance_list); ++i)
        query_writer_free(query_outputs[i]);

DEFER_5:
    free(query_outputs);
DEFER_4:
    database_free(database);
DEFER_3:
    query_instance_list_free(query_instance_list, query_type_list);
DEFER_2:
    fclose(query_file);
DEFER_1:
    query_type_list_free(query_type_list);
DEFER_0:
    return retval;
}
