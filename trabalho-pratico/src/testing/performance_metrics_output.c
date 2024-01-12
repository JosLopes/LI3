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
 * @brief Implementation of methods in include/testing/performance_metrics_output.h
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_metrics_output_example).
 */

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <wchar.h>

#include "interactive_mode/ncurses_utils.h"
#include "queries/query_type_list.h"
#include "testing/performance_metrics_output.h"
#include "utils/int_utils.h"
#include "utils/table.h"

/**
 * @brief Calulates which unit should be used to display data.
 *
 * @param n             Number of data points.
 * @param data          Data points.
 * @param unit_dates    Name of the units to choose from. Must increase in factors of 1000 (e.g.:
 *                      `{"us", "ms", "s"}`).
 * @param out_unit_name Where to output the name of the chosen unit.
 *
 * @return The value to divide each data point by, in order to get the data point in the chosen
 *         unit. This value will be `1`, `2`, or `3`.
 */
int __performance_metrics_choose_unit(size_t          n,
                                      const uint64_t *data,
                                      const char     *unit_names[3],
                                      const char    **out_unit_name) {
    uint64_t avg = 0;
    for (size_t i = 0; i < n; ++i)
        avg += data[i];

    if (n != 0)
        avg /= n;
    avg = max(1, avg); /* Don't fail on log10(0) */

    int unit_multiplier_log = floor(log10(avg) / 3);
    if (unit_multiplier_log > 2)
        unit_multiplier_log = 2;
    *out_unit_name = unit_names[unit_multiplier_log];
    return pow(1000, unit_multiplier_log);
}

/**
 * @brief Calculates the units that should be used to present data in @p events.
 *
 * @param n                    Number of performance events in @p events.
 * @param events               Set of performance events to be presented. Some may be `NULL`, but
 *                             must be counted toweards @p n.
 * @param time_unit_multiplier Where to write the factor to divide time units.
 * @param mem_unit_multiplier  Where to write the factor to divide memory units.
 * @param time_unit_name       Where to write the name of the time unit to.
 * @param mem_unit_name        Where to write the name of the memory unit to.
 */
void __performance_metrics_output_choose_units_from_performance_events(
    size_t                      n,
    const performance_event_t **events,
    int                        *time_unit_multiplier,
    int                        *mem_unit_multiplier,
    const char                **time_unit_name,
    const char                **mem_unit_name) {

    uint64_t *times = malloc(n * sizeof(uint64_t)), *mems = malloc(n * sizeof(uint64_t));
    size_t    count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (events[i]) {
            times[count] = performance_event_get_elapsed_time(events[i]);
            mems[count]  = performance_event_get_used_memory(events[i]);
            ++count;
        }
    }

    const char *time_unit_names[3] = {"us", "ms", "s"};
    const char *mem_unit_names[3]  = {"KiB", "MiB", "GiB"};

    *time_unit_multiplier =
        __performance_metrics_choose_unit(count, times, time_unit_names, time_unit_name);
    *mem_unit_multiplier =
        __performance_metrics_choose_unit(count, mems, mem_unit_names, mem_unit_name);

    free(times);
    free(mems);
}

/**
 * @brief Prints a table with performance events.
 *
 * @param output      Where to print the table to.
 * @param events      Events to be printed. Some may be `NULL`, but must be counted toweards @p n.
 * @param event_names Names of the events on the table.
 * @param n           Number of @p events and @p event_names.
 */
void __performance_metrics_output_print_table(FILE                       *output,
                                              const performance_event_t **events,
                                              const wchar_t             **event_names,
                                              size_t                      n) {

    /* Choose best units to fit data */
    int         time_unit_multiplier, mem_unit_multiplier;
    const char *time_unit_name, *mem_unit_name;
    __performance_metrics_output_choose_units_from_performance_events(n,
                                                                      events,
                                                                      &time_unit_multiplier,
                                                                      &mem_unit_multiplier,
                                                                      &time_unit_name,
                                                                      &mem_unit_name);

    table_t *table = table_create(n + 1, 3);

    wchar_t time[11];
    swprintf(time, 11, L"Time (%3s)", time_unit_name);

    wchar_t memory[13];
    swprintf(memory, 13, L"Memory (%3s)", mem_unit_name);

    table_insert_wide_string(table, time, 1, 0);
    table_insert_wide_string(table, memory, 2, 0);

    for (size_t i = 0; i < n; i++) {
        table_insert_wide_string(table, event_names[i], 0, i + 1);

        const performance_event_t *perf = events[i];
        if (!perf)
            continue;

        table_insert_double(table,
                            (double) performance_event_get_elapsed_time(perf) /
                                time_unit_multiplier,
                            1,
                            i + 1);
        table_insert_double(table,
                            (double) performance_event_get_used_memory(perf) / mem_unit_multiplier,
                            2,
                            i + 1);
    }

    table_draw(output, table);
    table_free(table);
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

/**
 * @brief Prints performance information about execution of all queries of the same type.
 *
 * @param output          Stream where to output data.
 * @param n               Number of @p line_numbers and @p times.
 * @param line_numbers    Numbers of the lines where queries of a given type occurred.
 * @param times           Times (us) it took to process each query (by line number).
 * @param statistics_time Time it took to generate query statistics.
 */
void __performance_metrics_output_query(FILE           *output,
                                        size_t          n,
                                        const size_t   *line_numbers,
                                        const uint64_t *times,
                                        uint64_t        statistics_time) {
    if (n == 0)
        return;

    /* Calulate amortized times */
    uint64_t *amortized = malloc(n * sizeof(uint64_t));
    if (!amortized)
        return;

    for (size_t i = 0; i < n; ++i) {
        amortized[i] = times[i] + statistics_time / n;
    }

    const char *units[3] = {"us", "ms", "s"};

    /* Choose best units to fit data */
    int         time_unit_multiplier, amortized_unit_multiplier;
    const char *time_unit_name, *amortized_unit_name;

    time_unit_multiplier = __performance_metrics_choose_unit(n, times, units, &time_unit_name);
    amortized_unit_multiplier =
        __performance_metrics_choose_unit(n, amortized, units, &amortized_unit_name);

    table_t *table = table_create(n + 1, 3);

    wchar_t time[11];
    swprintf(time, 11, L"Time (%3s)", time_unit_name);

    wchar_t amortized_time[16];
    swprintf(amortized_time, 16, L"Amortized (%3s)", amortized_unit_name);

    table_insert_wide_string(table, time, 1, 0);
    table_insert_wide_string(table, amortized_time, 2, 0);

    for (size_t i = 0; i < n; i++) {
        wchar_t current_line[11];
        swprintf(current_line, 11, L"Line %5zu", line_numbers[i]);
        table_insert_wide_string(table, current_line, 0, i + 1);

        table_insert_double(table, (double) times[i] / time_unit_multiplier, 1, i + 1);

        if (statistics_time)
            table_insert_double(table, (double) amortized[i] / amortized_unit_multiplier, 2, i + 1);
    }

    table_draw(output, table);
    table_free(table);
    free(amortized);
}

void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics) {
    if (output == stdout)
        fprintf(output, "\n\x1b[1;4mDATASET LOADING\x1b[22;24m\n\n");
    else
        fprintf(output, "\nDATASET LOADING\n\n");
    __performance_metrics_output_print_dataset(output, metrics);

    if (output == stdout)
        fprintf(output, "\n\x1b[1;4mQUERY STATISTICAL DATA GENERATION\x1b[22;24m\n\n");
    else
        fprintf(output, "\nQUERY STATISTICAL DATA GENERATION\n\n");
    __performance_metrics_output_print_query_statistics(output, metrics);

    if (output == stdout)
        fprintf(output, "\n\x1b[1;4mQUERY EXECUTION\x1b[22;24m\n\n");
    else
        fprintf(output, "\nQUERY EXECUTION\n");

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        size_t   *line_numbers;
        uint64_t *times;

        fprintf(output, "\nQuery %zu\n\n", i + 1);

        const performance_event_t *statistics_event =
            performance_metrics_get_query_statistics_measurement(metrics, i);
        uint64_t statistics_time =
            statistics_event ? performance_event_get_elapsed_time(statistics_event) : 0;

        ssize_t len =
            performance_metrics_get_query_execution_measurements(metrics, i, &line_numbers, &times);

        __performance_metrics_output_query(output, len, line_numbers, times, statistics_time);

        g_free(line_numbers);
        g_free(times);
    }

    putchar('\n');
}
