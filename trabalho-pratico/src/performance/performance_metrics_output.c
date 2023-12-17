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

#include <math.h>
#include <stdio.h>

#include "performance/performance_metrics_output.h"

/**
 * @brief Calculates the units that should be used to present data in @p dataset_events.
 *
 * @param dataset_events       Set of performance events to be presented.
 * @param n                    Number of performance events in @p dataset_events.
 * @param time_unit_multiplier Where to write the factor to divide time units to.
 * @param mem_unit_multiplier  Where to write the factor to divide memory units to.
 * @param time_unit_string     Where to write the name of the time unit to.
 * @param mem_unit_string      Where to write the name of the memory unit to.
 */
void __performance_metrics_output_choose_units(const performance_event_t **dataset_events,
                                               size_t                      n,
                                               int                        *time_unit_multiplier,
                                               int                        *mem_unit_multiplier,
                                               const char                **time_unit_string,
                                               const char                **mem_unit_string) {
    uint64_t time_avg = 0;
    size_t   mem_avg  = 0;

    for (size_t i = 0; i < n; ++i) {
        if (!dataset_events[i])
            continue;

        time_avg += performance_event_get_elapsed_time(dataset_events[i]);
        mem_avg += performance_event_get_used_memory(dataset_events[i]);
    }

    if (n != 0) {
        time_avg /= n;
        mem_avg /= n;
    }

    *time_unit_multiplier = time_avg != 0 ? pow(1000, floor(log10(time_avg) / 3)) : 1;
    if (*time_unit_multiplier > 1000000)
        *time_unit_multiplier = 1000000;

    switch (*time_unit_multiplier) {
        case 1:
            *time_unit_string = "us";
            break;
        case 1000:
            *time_unit_string = "ms";
            break;
        case 1000000:
            *time_unit_string = "s";
            break;
        default:
            *time_unit_string = "?";
            break;
    }

    *mem_unit_multiplier = mem_avg != 0 ? pow(1000, floor(log10(mem_avg) / 3)) : 1;
    if (*mem_unit_multiplier > 1000000)
        *mem_unit_multiplier = 1000000;

    switch (*mem_unit_multiplier) {
        case 1:
            *mem_unit_string = "KiB";
            break;
        case 1000:
            *mem_unit_string = "MiB";
            break;
        case 1000000:
            *mem_unit_string = "GiB";
            break;
        default:
            *mem_unit_string = "?";
            break;
    }
}

/**
 * @brief Prints the performance information about dataset loading in @p metrics to @p output.
 *
 * @param output  Stream where to output data.
 * @param metrics Performance metrics to be printed.
 */
void __performance_metrics_output_print_dataset(FILE                        *output,
                                                const performance_metrics_t *metrics) {

    /* Load events */
    const performance_event_t *dataset_events[PERFORMANCE_METRICS_DATASET_STEP_DONE];
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        const performance_event_t *perf = performance_metrics_get_dataset_measurement(metrics, i);
        if (perf)
            dataset_events[i] = perf;
        else
            dataset_events[i] = NULL;
    }

    /* Choose units */
    int         time_unit_multiplier, mem_unit_multiplier;
    const char *time_unit_string, *mem_unit_string;
    __performance_metrics_output_choose_units(dataset_events,
                                              PERFORMANCE_METRICS_DATASET_STEP_DONE,
                                              &time_unit_multiplier,
                                              &mem_unit_multiplier,
                                              &time_unit_string,
                                              &mem_unit_string);

    fprintf(output, "               +------------+--------------+\n");
    fprintf(output, "               |            |              |\n");
    fprintf(output,
            "               | Time (%3s) | Memory (%3s) |\n",
            time_unit_string,
            mem_unit_string);
    fprintf(output, "               |            |              |\n");
    fprintf(output, "+--------------+------------+--------------+\n");

    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        const performance_event_t *perf = dataset_events[i];

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

        fprintf(output, "|              |            |              |\n");
        fprintf(output,
                "| %12s | %10.2lf | %12.2lf |\n",
                where,
                (double) performance_event_get_elapsed_time(perf) / time_unit_multiplier,
                (double) performance_event_get_used_memory(perf) / mem_unit_multiplier);
        fprintf(output, "|              |            |              |\n");
        fprintf(output, "+--------------+------------+--------------+\n");
    }
}

void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics) {
    __performance_metrics_output_print_dataset(output, metrics);
}
