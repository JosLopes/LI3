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
 * @file    dataset_input.h
 * @brief   Module that reads all files from a dataset.
 * @details See [dataset_loader](@ref dataset_loader.h) for more information.
 *
 * There is a mandatory order in which `dataset_input_load_*` methods must be called, as some parts
 * of the dataset depend on others already being loaded. The correct order must be a valid
 * topological order of the following graph:
 *
 * @dot
 *     digraph {
 *         node [fontname = "monospace"];
 *         users   -> reservations
 *         users   -> passengers
 *         flights -> passengers
 *     }
 * @enddot
 *
 * Of course, some operations may happen in parallel if there are no data dependencies (e.g.:
 * loading `users` and `flights` simultaneously).
 *
 * @anchor dataset_input_examples
 * ### Example
 *
 * In order to load a dataset, this module should be used in conjunction with
 * [dataset_error_output](@ref dataset_error_output.h). See the source code of ::dataset_loader_load
 * for a good example on how to use both of these modules.
*/

#ifndef DATASET_INPUT_H
#define DATASET_INPUT_H

#include "database/database.h"
#include "dataset/dataset_error_output.h"

/** @brief Collection of file handles for all dataset input files. */
typedef struct dataset_input dataset_input_t;

/**
 * @brief  Attempts to open all file handles for dataset input files.
 * @param  path Path to the directory containing the files.
 * @return A collection of file handles that must be `free`'d with ::dataset_input_free, or `NULL`
 *         on IO / allocation error.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
dataset_input_t *dataset_input_create(const char *path);

/**
 * @brief Loads all the users in a dataset into a @p database.
 *
 * @param input    Collection of file handles for dataset input.
 * @param output   Collection of file handles for dataset error output.
 * @param database Database to load the new users into.
 *
 * @retval 0 Success.
 * @retval 1 Failure.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
int dataset_input_load_users(dataset_input_t        *input,
                             dataset_error_output_t *output,
                             database_t             *database);

/**
 * @brief Loads all the flights in a dataset into a @p database.
 *
 * @param input    Collection of file handles for dataset input.
 * @param output   Collection of file handles for dataset error output.
 * @param database Database to load the new flights into.
 *
 * @retval 0 Success.
 * @retval 1 Failure.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
int dataset_input_load_flights(dataset_input_t        *input,
                               dataset_error_output_t *output,
                               database_t             *database);

/**
 * @brief Loads all the user-flight relationships (passengers) in a dataset into a @p database.
 *
 * @param input    Collection of file handles for dataset input.
 * @param output   Collection of file handles for dataset error output.
 * @param database Database to load the new passengers into.
 *
 * @retval 0 Success.
 * @retval 1 Failure.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
int dataset_input_load_passengers(dataset_input_t        *input,
                                  dataset_error_output_t *output,
                                  database_t             *database);

/**
 * @brief Loads all the reservations in a dataset into a @p database.
 *
 * @param input    Collection of file handles for dataset input.
 * @param output   Collection of file handles for dataset error output.
 * @param database Database to load the new reservations into.
 *
 * @retval 0 Success.
 * @retval 1 Failure.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
int dataset_input_load_reservations(dataset_input_t        *input,
                                    dataset_error_output_t *output,
                                    database_t             *database);

/**
 * @brief Closes all file handles in @p input and `free`s the data structure.
 * @param input Value to be deleted, allocated by ::dataset_input_create.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */
void dataset_input_free(dataset_input_t *input);

#endif
