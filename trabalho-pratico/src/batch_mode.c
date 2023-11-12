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
 * @brief Implementation of methods in include/queries/batch_mode.h
 *
 * ### Examples
 * See [the header file's documentation](@ref batch_mode_examples).
 */

#include <limits.h>
#include <stdio.h>

#include "dataset/dataset_loader.h"
#include "queries/query_dispatcher.h"
#include "queries/query_file_parser.h"
#include "queries/query_type_list.h"

/**
 * @brief Data structure used for query iteration in ::__batch_mode_init_file_callback.
 *
 * @var batch_mode_iter_data::outputs
 *     @brief Where to write opened files to.
 * @var batch_mode_iter_data::i
 *     @brief Index of the query being currently dealt with.
 */
typedef struct {
    FILE **outputs;
    size_t i;
} batch_mode_iter_data;

/**
 * @brief Called for each query, to open the file to write the query output.
 *
 * @param user_data A pointer to a ::batch_mode_iter_data.
 * @param instance  Query query instance, whose output should be outputted.
 */
int __batch_mode_init_file_callback(void *user_data, query_instance_t *instance) {
    batch_mode_iter_data *iter_data = (batch_mode_iter_data *) user_data;

    char path[PATH_MAX];
    sprintf(path, "Resultados/command%zu_output.txt", query_instance_get_number_in_file(instance));

    iter_data->outputs[iter_data->i] = fopen(path, "w");
    if (!iter_data->outputs[iter_data->i]) {
        /* On failure, close all already-opened files */
        for (size_t j = 0; j < iter_data->i; ++j)
            fclose(iter_data->outputs[j]);
        return 1;
    }

    iter_data->i++;
    return 0;
}

int batch_mode_run(const char *dataset_dir, const char *query_file_path) {
    (void) query_file_path;

    query_type_list_t *query_type_list = query_type_list_create();
    if (!query_type_list) {
        fputs("Failed to allocate query definitions!\n", stderr);
        return 1;
    }

    FILE *query_file = fopen(query_file_path, "r");
    if (!query_file) {
        query_type_list_free(query_type_list);
        fputs("Failed to read query file!\n", stderr);
        return 1;
    }

    query_instance_list_t *query_instance_list =
        query_file_parser_parse(query_file, query_type_list);
    if (!query_instance_list) {
        query_type_list_free(query_type_list);
        fclose(query_file);
        fputs("Failed to allocate list of queries!\n", stderr);
        return 1;
    }

    database_t *database = database_create();
    if (!database) {
        query_instance_list_free(query_instance_list, query_type_list);
        query_type_list_free(query_type_list);
        fclose(query_file);
        fputs("Failed to allocate database!\n", stderr);
        return 1;
    }

    if (dataset_loader_load(database, dataset_dir)) {
        query_instance_list_free(query_instance_list, query_type_list);
        query_type_list_free(query_type_list);
        database_free(database);
        fclose(query_file);
        fputs("Failed to load dataset files!\n", stderr);
        return 1;
    }

    FILE **query_outputs =
        malloc(sizeof(FILE *) * query_instance_list_get_length(query_instance_list));
    if (!query_outputs) {
        query_instance_list_free(query_instance_list, query_type_list);
        query_type_list_free(query_type_list);
        database_free(database);
        fclose(query_file);
        fputs("Failed to allocate list of query outputs!\n", stderr);
        return 1;
    }

    batch_mode_iter_data iter_data = {.outputs = query_outputs, .i = 0};
    if (query_instance_list_iter(query_instance_list,
                                 __batch_mode_init_file_callback,
                                 &iter_data)) {

        query_instance_list_free(query_instance_list, query_type_list);
        query_type_list_free(query_type_list);
        database_free(database);
        fclose(query_file);
        free(query_outputs);
        fputs("Failed to open one of the query outputs!\n", stderr);
        return 1;
    }

    query_dispatcher_dispatch_list(database, query_instance_list, query_type_list, query_outputs);

    for (size_t i = 0; i < query_instance_list_get_length(query_instance_list); ++i)
        fclose(query_outputs[i]);
    free(query_outputs);

    query_instance_list_free(query_instance_list, query_type_list);
    query_type_list_free(query_type_list);
    database_free(database);
    fclose(query_file);
    return 0;
}
