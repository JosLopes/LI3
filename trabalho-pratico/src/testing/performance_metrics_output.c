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
 * ### Examples
 * See [the header file's documentation](@ref performance_metrics_output_example).
 */

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "queries/query_type_list.h"
#include "testing/performance_metrics_output.h"
#include "utils/table.h"

/**
 * @brief Calulates which unit should be used to display a set of data points.
 *
 * @param n             Number of data points in @p data.
 * @param data          Data points.
 * @param unit_names    Name of the units to choose from. These must increase in factors of 1000
 *                      (e.g.: `{"us", "ms", "s"}`).
 * @param out_unit_name Where to output the name of the chosen unit to.
 *
 * @return The value to divide each data point by, in order to get the data point in the chosen
 *         unit. This value will be either `1`, `1000`, or `1 000 000`.
 */
int __performance_metrics_choose_unit(size_t            n,
                                      const uint64_t    data[n],
                                      const char *const unit_names[3],
                                      const char      **out_unit_name) {
    uint64_t avg = 0;
    for (size_t i = 0; i < n; ++i)
        avg += data[i];

    if (n != 0)
        avg /= n;
    if (avg < 1)
        avg = 1;

    const int unit_multiplier_log = fmin(floor(log10(avg) / 3), 2);
    *out_unit_name                = unit_names[unit_multiplier_log];
    return pow(1000, unit_multiplier_log);
}

/**
 * @brief Calculates the units that should be used to present data in a set of
 *        ::performance_event_t.
 *
 * @param n                    Number of performance events in @p events.
 * @param events               Set of performance events to be presented. Some may be `NULL`, but
 *                             must be counted towards @p n.
 * @param time_unit_multiplier Where to write the division factor of time units to.
 * @param mem_unit_multiplier  Where to write the division factor of memory units to.
 * @param time_unit_name       Where to write the name of the time unit to.
 * @param mem_unit_name        Where to write the name of the memory unit to.
 */
void __performance_metrics_output_choose_units_from_performance_events(
    size_t                           n,
    const performance_event_t *const events[n],
    int                             *time_unit_multiplier,
    int                             *mem_unit_multiplier,
    const char                     **time_unit_name,
    const char                     **mem_unit_name) {

    uint64_t *const times = malloc(n * sizeof(uint64_t));
    uint64_t *const mems  = malloc(n * sizeof(uint64_t));
    if (!times || !mems) {
        if (times)
            free(times);
        if (mems)
            free(mems);

        /* Fill units with values before failing, not to cause invalid reads. */
        *time_unit_multiplier = 1;
        *mem_unit_multiplier  = 1;
        *time_unit_name       = "?";
        *mem_unit_name        = "?";
        return;
    }

    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (events[i]) {
            times[count] = performance_event_get_elapsed_time(events[i]);
            mems[count]  = performance_event_get_used_memory(events[i]);
            ++count;
        }
    }

    const char *const time_unit_names[] = {"us", "ms", "s"};
    const char *const mem_unit_names[]  = {"KiB", "MiB", "GiB"};

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
 * @param n           Number of @p events and @p event_names.
 * @param events      Events to be printed. Some may be `NULL`, but must be counted towards @p n.
 * @param event_names Names of the events on the table.
 */
void __performance_metrics_output_print_table(FILE                            *output,
                                              size_t                           n,
                                              const performance_event_t *const events[n],
                                              const char *const                event_names[n]) {
    /* Choose best units to fit data */
    int         time_unit_multiplier, mem_unit_multiplier;
    const char *time_unit_name, *mem_unit_name;
    __performance_metrics_output_choose_units_from_performance_events(n,
                                                                      events,
                                                                      &time_unit_multiplier,
                                                                      &mem_unit_multiplier,
                                                                      &time_unit_name,
                                                                      &mem_unit_name);
    table_t *const table = table_create(3, n + 1);
    if (!table)
        return;
    table_insert_format(table, 1, 0, "Time (%s)", time_unit_name);
    table_insert_format(table, 2, 0, "Memory (%s)", mem_unit_name);

    for (size_t i = 0; i < n; i++) {
        table_insert_format(table, 0, i + 1, "%s", event_names[i]);

        const performance_event_t *const perf = events[i];
        if (!perf)
            continue;

        table_insert_format(table,
                            1,
                            i + 1,
                            "%.2lf",
                            (double) performance_event_get_elapsed_time(perf) /
                                time_unit_multiplier);

        table_insert_format(table,
                            2,
                            i + 1,
                            "%.2lf",
                            (double) performance_event_get_used_memory(perf) / mem_unit_multiplier);
    }

    table_draw(output, table);
    table_free(table);
}

/**
 * @brief Prints the performance information about dataset loading in @p metrics.
 *
 * @param output  Stream where to output formatted performance data to.
 * @param metrics Performance metrics to extract dataset information from.
 *
 * @return The time (in microseconds) it took to load the dataset.
 */
uint64_t __performance_metrics_output_print_dataset(FILE                        *output,
                                                    const performance_metrics_t *metrics) {
    uint64_t ret = 0;

    const performance_event_t *dataset_events[PERFORMANCE_METRICS_DATASET_STEP_DONE];
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        dataset_events[i] = performance_metrics_get_dataset_measurement(metrics, i);
        if (dataset_events[i])
            ret += performance_event_get_elapsed_time(dataset_events[i]);
    }

    const char *const event_names[] = {"Users", "Flights", "Passengers", "Reservations"};
    __performance_metrics_output_print_table(output, 4, dataset_events, event_names);
    return ret;
}

/**
 * @brief Prints the performance information about query statistical data generation in @p metrics.
 *
 * @param output  Stream where to output formatted performance data to.
 * @param metrics Performance metrics to extract query statistical data generation information from.
 *
 * @return The time (in microseconds) it took to generate query statistics (for all queries).
 */
uint64_t __performance_metrics_output_print_query_statistics(FILE                        *output,
                                                             const performance_metrics_t *metrics) {
    uint64_t ret = 0;

    const performance_event_t *statistical_events[QUERY_TYPE_LIST_COUNT];
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        statistical_events[i] =
            performance_metrics_get_query_statistics_measurement(metrics, i + 1);

        if (statistical_events[i])
            ret += performance_event_get_elapsed_time(statistical_events[i]);
    }

    char *event_names[QUERY_TYPE_LIST_COUNT];
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        event_names[i] = malloc(sizeof(char) * 32);
        snprintf(event_names[i], 32, "Query %zu", i + 1);
    }

    __performance_metrics_output_print_table(output,
                                             QUERY_TYPE_LIST_COUNT,
                                             statistical_events,
                                             (const char *const *) event_names);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        free(event_names[i]);

    return ret;
}

/**
 * @brief Prints performance information about the execution of all queries of the same type.
 *
 * @param output          Stream where to output formatted performance data to.
 * @param query_type      Type of the query the performance information relates to.
 * @param n               Number of @p line_numbers and @p times.
 * @param line_numbers    Numbers of the lines where queries of a given type occurred.
 * @param times           Times (in microseconds) it took to process each query.
 * @param statistics_time Time it took to generate query statistical data. Leave `0` to inform that
 *                        @p query_type doesn't generate statistical data.
 *
 * @return The time (in microseconds) it took to execute all queries of type @p query_type (doesn't
 *         include statistical data generation).
 */
uint64_t __performance_metrics_output_print_query(FILE          *output,
                                                  const size_t   query_type,
                                                  size_t         n,
                                                  const size_t   line_numbers[n],
                                                  const uint64_t times[n],
                                                  uint64_t       statistics_time) {
    uint64_t ret = 0;

    if (n == 0)
        return 0;
    fprintf(output, "\nQuery %zu\n\n", query_type);

    /* Calulate amortized times */
    uint64_t *const amortized = malloc(n * sizeof(uint64_t));
    if (!amortized)
        return 0;
    for (size_t i = 0; i < n; ++i) {
        ret += times[i];
        amortized[i] = times[i] + statistics_time / n;
    }

    /* Choose best units to fit data */
    const char *const units[3] = {"us", "ms", "s"};
    int               time_unit_multiplier, amortized_unit_multiplier;
    const char       *time_unit_name, *amortized_unit_name;
    time_unit_multiplier = __performance_metrics_choose_unit(n, times, units, &time_unit_name);
    amortized_unit_multiplier =
        __performance_metrics_choose_unit(n, amortized, units, &amortized_unit_name);

    /* Print table */
    table_t *const table = table_create(3, n + 1);
    if (!table) {
        free(amortized);
        return ret;
    }
    table_insert_format(table, 1, 0, "Time (%s)", time_unit_name);
    table_insert_format(table, 2, 0, "Amortized (%s)", amortized_unit_name);

    for (size_t i = 0; i < n; i++) {
        table_insert_format(table, 0, i + 1, "Line %5zu", line_numbers[i]);
        table_insert_format(table, 1, i + 1, "%.2lf", (double) times[i] / time_unit_multiplier);

        if (statistics_time)
            table_insert_format(table,
                                2,
                                i + 1,
                                "%.2lf",
                                (double) amortized[i] / amortized_unit_multiplier);
    }

    table_draw(output, table);
    table_free(table);
    free(amortized);

    return ret;
}

/**
 * @brief Prints performance information about the execution of all queries in batch mode's input.
 *
 * @param output  Stream where to output formatted performance data to.
 * @param metrics Performance metrics to extract query execution information from.
 *
 * @return The time (in microseconds) it took to execute all queries (doesn't include statistical
 *         data generation).
 */
uint64_t __performance_metrics_output_print_all_queries(FILE                        *output,
                                                        const performance_metrics_t *metrics) {
    uint64_t ret = 0;
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        size_t   *line_numbers;
        uint64_t *times;

        const performance_event_t *const statistics_event =
            performance_metrics_get_query_statistics_measurement(metrics, i + 1);
        const uint64_t statistics_time =
            statistics_event ? performance_event_get_elapsed_time(statistics_event) : 0;

        const size_t len = performance_metrics_get_query_execution_measurements(metrics,
                                                                                i + 1,
                                                                                &line_numbers,
                                                                                &times);
        ret += __performance_metrics_output_print_query(output,
                                                        i + 1,
                                                        len,
                                                        line_numbers,
                                                        times,
                                                        statistics_time);
        free(line_numbers);
        free(times);
    }
    return ret;
}

/**
 * @brief Prints a summary of the performance data collected.
 *
 * @param output       Stream where to output formatted performance data to.
 * @param metrics      Performance metrics to extract query execution information from.
 * @param dataset_time Time (in microseconds) it took to load the dataset.
 * @param query_time   Time (in microseconds) it took to execute all queries (including statistical
 *                     data generation).
 */
void __performance_metrics_output_summary(FILE                        *output,
                                          const performance_metrics_t *metrics,
                                          uint64_t                     dataset_time,
                                          uint64_t                     query_time) {

    const char *const time_unit_names[3] = {"us", "ms", "s"};
    const char *const mem_unit_names[3]  = {"KiB", "MiB", "GiB"};
    const char       *unit_name;
    int               multiplier;

    const uint64_t total_time = performance_metrics_get_program_total_time(metrics);
    multiplier = __performance_metrics_choose_unit(1, &total_time, time_unit_names, &unit_name);
    fprintf(output, "Total time: %6.2lf %2s\n", (double) total_time / multiplier, unit_name);

    multiplier = __performance_metrics_choose_unit(1, &dataset_time, time_unit_names, &unit_name);
    fprintf(output,
            "   Dataset: %6.2lf %2s (%4.1f %%)\n",
            (double) dataset_time / multiplier,
            unit_name,
            (dataset_time * 100.0) / total_time);

    multiplier = __performance_metrics_choose_unit(1, &query_time, time_unit_names, &unit_name);
    fprintf(output,
            "   Queries: %6.2lf %2s (%4.1f %%)\n",
            (double) query_time / multiplier,
            unit_name,
            (query_time * 100.0) / total_time);

    const uint64_t total_mem = performance_metrics_get_program_total_mem(metrics);
    multiplier = __performance_metrics_choose_unit(1, &total_mem, mem_unit_names, &unit_name);
    fprintf(output, "\nPeak memory: %.2lf %s\n", (double) total_mem / multiplier, unit_name);
}

void performance_metrics_output_print(FILE *output, const performance_metrics_t *metrics) {
    /* To know if ANSI escape codes for bold and underline can be used. */
    const int tty = isatty(fileno(output));

    if (tty)
        fprintf(output, "\n\x1b[1;4mDATASET LOADING\x1b[22;24m\n\n");
    else
        fprintf(output, "\nDATASET LOADING\n\n");
    const uint64_t dataset_time = __performance_metrics_output_print_dataset(output, metrics);

    if (tty)
        fprintf(output, "\n\x1b[1;4mQUERY STATISTICAL DATA GENERATION\x1b[22;24m\n\n");
    else
        fprintf(output, "\nQUERY STATISTICAL DATA GENERATION\n\n");
    uint64_t query_time = __performance_metrics_output_print_query_statistics(output, metrics);

    if (tty)
        fprintf(output, "\n\x1b[1;4mQUERY EXECUTION\x1b[22;24m\n\n");
    else
        fprintf(output, "\nQUERY EXECUTION\n\n");
    query_time += __performance_metrics_output_print_all_queries(output, metrics);

    if (tty)
        fprintf(output, "\n\x1b[1;4mPERFORMANCE SUMMARY\x1b[22;24m\n\n");
    else
        fprintf(output, "\nPERFORMANCE SUMMARY\n\n");

    __performance_metrics_output_summary(output, metrics, dataset_time, query_time);
    putchar('\n');
}
