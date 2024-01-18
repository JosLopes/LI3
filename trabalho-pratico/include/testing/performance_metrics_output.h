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
 * @file  performance_metrics_output.h
 * @brief Method to produce formatted output from data in ::performance_metrics_t.
 *
 * @anchor performance_metrics_output_example
 * ### Example
 *
 * See test.c to know how to create a ::performance_metrics_t structure. Then, just call
 * ::performance_metrics_output_print with an output file (usually `stdout`).
 */

#ifndef PERFORMANCE_OUTPUT_H
#define PERFORMANCE_OUTPUT_H

#include <stdio.h>

#include "testing/performance_metrics.h"

/**
 * @brief Prints data in @p metrics to @p output.
 *
 * @param output  Stream where to output data.
 * @param metrics Performance metrics to be printed.
 */
void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics);

#endif
