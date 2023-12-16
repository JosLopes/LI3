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
 * @file  performance_metrics_output.c
 * @brief Implementation of methods in include/performance/performance_metrics_output.h
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_metrics_output_example).
 */

#include <stdio.h>

#include "performance/performance_metrics_output.h"

/**
 * @brief Prints the performance information about dataset loading in @p metrics to @p output.
 *
 * @param output  Stream where to output data.
 * @param metrics Performance metrics to be printed.
 */
void __performance_metrics_output_print_dataset(FILE                        *output,
                                                const performance_metrics_t *metrics) {

    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        const performance_event_t *perf = performance_metrics_get_dataset_measurement(metrics, i);
        if (!perf)
            continue;

        const char *where;
        switch (i) {
            case PERFORMANCE_METRICS_DATASET_STEP_USERS:
                where = "Users";
                break;
            case PERFORMANCE_METRICS_DATASET_STEP_FLIGHTS:
                where = "Flights";
                break;
            case PERFORMANCE_METRICS_DATASET_STEP_PASSENGERS:
                where = "Passengers";
                break;
            case PERFORMANCE_METRICS_DATASET_STEP_RESERVATIONS:
                where = "Reservations";
                break;
            default:
                where = NULL;
                break;
        };

        fprintf(output,
                "%s\nTime: %" PRIu64 " us\nMemory: %zu KiB\n\n",
                where,
                performance_event_get_elapsed_time(perf),
                performance_event_get_used_memory(perf));
    }
}

void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics) {
    __performance_metrics_output_print_dataset(output, metrics);
}
