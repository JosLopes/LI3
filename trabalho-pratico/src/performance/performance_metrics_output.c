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

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <wchar.h>

#include "interactive_mode/ncurses_utils.h"
#include "performance/performance_metrics_output.h"
#include "queries/query_type_list.h"
#include "utils/int_utils.h"

/**
 * @brief Calculates the units that should be used to present data in @p events.
 *
 * @param events               Set of performance events to be presented.
 * @param n                    Number of performance events in @p events.
 * @param time_unit_multiplier Where to write the factor to divide time units.
 * @param mem_unit_multiplier  Where to write the factor to divide memory units.
 * @param time_unit_string     Where to write the name of the time unit to.
 * @param mem_unit_string      Where to write the name of the memory unit to.
 */
void __performance_metrics_output_choose_units(const performance_event_t **events,
                                               size_t                      n,
                                               int                        *time_unit_multiplier,
                                               int                        *mem_unit_multiplier,
                                               const char                **time_unit_string,
                                               const char                **mem_unit_string) {
    /* Calculate time and memory averages */
    uint64_t time_avg = 0;
    size_t   mem_avg  = 0;
    for (size_t i = 0; i < n; ++i) {
        if (!events[i])
            continue;

        time_avg += performance_event_get_elapsed_time(events[i]);
        mem_avg += performance_event_get_used_memory(events[i]);
    }

    if (n != 0) {
        time_avg /= n;
        mem_avg /= n;
    }
    time_avg = max(1, time_avg);
    mem_avg  = max(1, mem_avg);

    /* Choose correct units */
    const char *time_units[] = {"us", "ms", "s"};
    const char *mem_units[]  = {"KiB", "MiB", "GiB"};

    int unit_multiplier_log = round(log10(time_avg) / 3);
    if (unit_multiplier_log > 2)
        unit_multiplier_log = 2;
    *time_unit_string     = time_units[unit_multiplier_log];
    *time_unit_multiplier = pow(1000, unit_multiplier_log);

    unit_multiplier_log = round(log10(time_avg) / 3);
    if (unit_multiplier_log > 2)
        unit_multiplier_log = 2;
    *mem_unit_string     = mem_units[unit_multiplier_log];
    *mem_unit_multiplier = pow(1000, unit_multiplier_log);
}

/**
 * @brief Prints a table with performance events.
 *
 * @param output      Where to print the table to.
 * @param events      Events to be printed.
 * @param event_names Names of the events on the table.
 * @param n           Number of @p events and @p event_names.
 */
void __performance_metrics_output_print_table(FILE                       *output,
                                              const performance_event_t **events,
                                              const wchar_t             **event_names,
                                              size_t                      n) {

    /* Choose best units to fit data */
    int         time_unit_multiplier, mem_unit_multiplier;
    const char *time_unit_string, *mem_unit_string;
    __performance_metrics_output_choose_units(events,
                                              n,
                                              &time_unit_multiplier,
                                              &mem_unit_multiplier,
                                              &time_unit_string,
                                              &mem_unit_string);

    /* Calculate table measurements */
    int left_width = 0;
    for (size_t i = 0; i < n; ++i) {
        int width = ncurses_measure_unicode_string((gunichar *) event_names[i]);
        if (width > left_width)
            left_width = width;
    }

    char *left_hyphens = malloc(left_width + 1);
    memset(left_hyphens, '-', left_width);
    left_hyphens[left_width] = '\0';

    /* Actually print table */
    fprintf(output, "  %*s +------------+--------------+\n", left_width, "");
    fprintf(output, "  %*s |            |              |\n", left_width, "");
    fprintf(output,
            "  %*s | Time (%3s) | Memory (%3s) |\n",
            left_width,
            "",
            time_unit_string,
            mem_unit_string);
    fprintf(output, "  %*s |            |              |\n", left_width, "");
    fprintf(output, "+-%s-+------------+--------------+\n", left_hyphens);

    for (size_t i = 0; i < n; ++i) {
        const performance_event_t *perf = events[i];
        if (!perf)
            continue;

        fprintf(output, "| %*s |            |              |\n", left_width, "");
        fprintf(output,
                "| %*ls | %10.2lf | %12.2lf |\n",
                left_width,
                event_names[i],
                (double) performance_event_get_elapsed_time(perf) / time_unit_multiplier,
                (double) performance_event_get_used_memory(perf) / mem_unit_multiplier);
        fprintf(output, "| %*s |            |              |\n", left_width, "");
        fprintf(output, "+-%s-+------------+--------------+\n", left_hyphens);
    }

    free(left_hyphens);
}

/**
 * @brief Prints the performance information about dataset loading in @p metrics to @p output.
 *
 * @param output  Stream where to output data.
 * @param metrics Performance metrics to be printed.
 */
void __performance_metrics_output_print_dataset(FILE                        *output,
                                                const performance_metrics_t *metrics) {

    const performance_event_t *dataset_events[PERFORMANCE_METRICS_DATASET_STEP_DONE];
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        const performance_event_t *perf = performance_metrics_get_dataset_measurement(metrics, i);
        if (perf)
            dataset_events[i] = perf;
        else
            dataset_events[i] = NULL;
    }

    const wchar_t *event_names[] = {L"Users", L"Flights", L"Passengers", L"Reservations"};
    __performance_metrics_output_print_table(output, dataset_events, event_names, 4);
}

/**
 * @brief Prints the performance information about query statistical data generation in @p metrics
 *        to @p output.
 *
 * @param output  Stream where to output data.
 * @param metrics Performance metrics to be printed.
 */
void __performance_metrics_output_print_query_statistics(FILE                        *output,
                                                         const performance_metrics_t *metrics) {
    /* Load events */
    const performance_event_t *statistical_events[QUERY_TYPE_LIST_COUNT];
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        const performance_event_t *perf =
            performance_metrics_get_query_statistics_measurement(metrics, i);

        if (perf)
            statistical_events[i] = perf;
        else
            statistical_events[i] = NULL;
    }

    wchar_t *event_names[QUERY_TYPE_LIST_COUNT];
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        event_names[i] = malloc(sizeof(wchar_t) * 32);
        swprintf(event_names[i], 32, L"Query %zu", i);
    }

    __performance_metrics_output_print_table(output,
                                             statistical_events,
                                             (const wchar_t **) event_names,
                                             QUERY_TYPE_LIST_COUNT);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        free(event_names[i]);
}

void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics) {
    if (output == stdout)
        printf("\n\x1b[1;4mDATASET LOADING\x1b[22;24m\n\n");
    else
        printf("\nDATASET LOADING\n\n");
    __performance_metrics_output_print_dataset(output, metrics);

    if (output == stdout)
        printf("\n\x1b[1;4mQUERY STATISTICAL DATA GENERATION\x1b[22;24m\n\n");
    else
        printf("\nQUERY STATISTICAL DATA GENERATION\n\n");
    __performance_metrics_output_print_query_statistics(output, metrics);

    putchar('\n');
}
