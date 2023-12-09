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
 * @file  dataset_loader.c
 * @brief Implementation of methods in include/dataset/dataset_loader.h
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_loader_examples).
 */

#include <stdio.h>

#include "dataset/dataset_input.h"
#include "dataset/dataset_loader.h"

int dataset_loader_load(database_t *database, const char *path) {
    dataset_input_t *input_files = dataset_input_create(path);
    if (!input_files)
        return 1;

    /* TODO - change hardcoded */
    dataset_error_output_t *error_files = dataset_error_output_create("Resultados");
    if (!error_files) {
        dataset_input_free(input_files);
        return 1;
    }

    /* Load dataset */
    int retval = 0;

    if (dataset_input_load_users(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    if (dataset_input_load_flights(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    if (dataset_input_load_passengers(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

    if (dataset_input_load_reservations(input_files, error_files, database)) {
        retval = 1;
        goto CLEANUP;
    }

CLEANUP:
    dataset_input_free(input_files);
    dataset_error_output_free(error_files);
    return retval;
}
