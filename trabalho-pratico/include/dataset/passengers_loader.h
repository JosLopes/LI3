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
 * @file  passengers_loader.h
 * @brief Module to load all the passengers in a dataset into the database.
 */

#ifndef PASSENGERS_LOADER_H
#define PASSENGERS_LOADER_H

#include <stdio.h>

#include "database/database.h"
#include "dataset/dataset_error_output.h"

/**
 * @brief Parses a `passengers.csv` dataset file.
 *
 * @param passengers_stream File stream with passenger data to be loaded. It is assumed this stream
 *                          is ordered by flight identifier.
 * @param flights_stream    File stream with flight data to be printed in case of errors. It is
 *                          assumed this stream is also ordered by flight identifier.
 * @param database          Database to add users-flight relations (passengers) to.
 * @param output            Where to output dataset errors to.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int passengers_loader_load(FILE                   *passengers_stream,
                           FILE                   *flights_stream,
                           database_t             *database,
                           dataset_error_output_t *output);

#endif
