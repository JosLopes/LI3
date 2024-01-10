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
 * @file    dataset_loader.h
 * @brief   Module to load all the files in a dataset into the database.
 * @details A dataset, as specified in the project's requirements, is constituted by the following
 *          files:
 *
 *          - `users.csv`;
 *          - `flights.csv`;
 *          - `passengers.csv`;
 *          - `reservations.csv`.
 *
 * @anchor dataset_loader_examples
 * ### Example
 *
 * For an example on ::dataset_loader_load, see the [database.h header](@ref database_examples).
 */

#ifndef DATASET_LOADER_H
#define DATASET_LOADER_H

#include "database/database.h"
#include "testing/performance_metrics.h"

/**
 * @brief Parses a dataset in @p path and stores the data in @p database.
 *
 * @param database     Database where to store the dataset data in.
 * @param dataset_path Path to the directory containing the dataset.
 * @param errors_path  Path to the directory where to output error files to.
 * @param metrics      Where to register program performance data to. Can be `NULL` for no
 *                     profiling.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (this loader only fatally fails on IO errors).
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_loader_examples).
 */
int dataset_loader_load(database_t            *database,
                        const char            *dataset_path,
                        const char            *errors_path,
                        performance_metrics_t *metrics);

#endif
