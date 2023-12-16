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
 * @file  batch_mode.h
 * @brief Batch mode (read a batch of queries from a file)
 *
 * @anchor batch_mode_examples
 * ### Examples
 * See main.c.
 */

#ifndef BATCH_MODE_H
#define BATCH_MODE_H

#include "performance/performance_metrics.h"

/**
 * @brief Starts batch mode.
 *
 * @param dataset_dir     Path to the directory containing the dataset.
 * @param query_file_path Path to the file containing the queries
 * @param metrics         Where to register program performance data to. Can be `NULL` for no
 *                        profiling.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (allocation / file IO errors). A message will also be printed to
 *         `stderr`.
 *
 * #### Examples
 * See [the header file's documentation](@ref batch_mode_examples).
 */
int batch_mode_run(const char            *dataset_dir,
                   const char            *query_file_path,
                   performance_metrics_t *metrics);

#endif
