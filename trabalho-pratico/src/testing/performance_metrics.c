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
 * @file  performance_metrics.c
 * @brief Implementation of methods in include/testing/performance_metrics.h
 *
 * ### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "queries/query_type_list.h"
#include "testing/performance_metrics.h"

/**
 * @struct performance_metrics
 * @brief  Information about performance about different parts of the application.
 *
 * @var performance_metrics::current_dataset_step
 *     @brief Current part of the dataset being loaded.
 * @var performance_metrics::dataset_events
 *     @brief Performance information about dataset loading.
 * @var performance_metrics::statistical_events
 *     @brief Performance information about query statistical data collection.
 * @var performance_metrics::query_events
 *     @brief   Performance information about individual query execution.
 *     @details Hash tables that associate a query's line number in a file (integer) to a
 *              ::performance_event_t.
 * @var performance_metrics::program_total_time
 *     @brief Time (in microseconds) that the whole program took to be executed.
 * @var performance_metrics::program_total_mem
 *     @brief Peak memory usage (in KiB) of the program.
 */
struct performance_metrics {
    performance_metrics_dataset_step_t current_dataset_step;
    performance_event_t               *dataset_events[PERFORMANCE_METRICS_DATASET_STEP_DONE];
    performance_event_t               *statistical_events[QUERY_TYPE_LIST_COUNT];
    GHashTable                        *query_events[QUERY_TYPE_LIST_COUNT];

    uint64_t program_total_time;
    size_t   program_total_mem;
};

performance_metrics_t *performance_metrics_create(void) {
    performance_metrics_t *const ret = malloc(sizeof(performance_metrics_t));
    if (!ret)
        return NULL;

    ret->current_dataset_step = PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED;
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i)
        ret->dataset_events[i] = NULL;

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        ret->statistical_events[i] = NULL;
        ret->query_events[i]       = g_hash_table_new_full(g_direct_hash,
                                                     g_direct_equal,
                                                     NULL,
                                                     (GDestroyNotify) performance_event_free);
    }

    ret->program_total_time = 0;
    ret->program_total_mem  = 0;

    return ret;
}

performance_metrics_t *performance_metrics_clone(const performance_metrics_t *metrics) {
    performance_metrics_t *const ret = malloc(sizeof(performance_metrics_t));
    if (!ret)
        return NULL;
    memset(ret, 0, sizeof(performance_metrics_t)); /* To ease cleanup on allocation failure */

    ret->current_dataset_step = metrics->current_dataset_step;
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i) {
        if (metrics->dataset_events[i]) {

            ret->dataset_events[i] = performance_event_clone(metrics->dataset_events[i]);
            if (!ret->dataset_events[i]) { /* Allocation failure */
                performance_metrics_free(ret);
                return NULL;
            }
        }
    }

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i) {
        if (metrics->statistical_events[i]) {

            ret->statistical_events[i] = performance_event_clone(metrics->statistical_events[i]);
            if (!ret->statistical_events[i]) { /* Allocation failure */
                performance_metrics_free(ret);
                return NULL;
            }
        }

        /* glib doesn't have a hash table cloning method. Go figure! */
        ret->query_events[i] = g_hash_table_new_full(g_direct_hash,
                                                     g_direct_equal,
                                                     NULL,
                                                     (GDestroyNotify) performance_event_free);
        GHashTableIter iter;
        gpointer       key, value;
        g_hash_table_iter_init(&iter, metrics->query_events[i]);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            performance_event_t *const event_clone = performance_event_clone(value);
            if (!event_clone) {
                performance_metrics_free(ret);
                return NULL;
            }

            g_hash_table_insert(ret->query_events[i], key, event_clone);
        }
    }

    ret->program_total_time = metrics->program_total_time;
    ret->program_total_mem  = metrics->program_total_mem;

    return ret;
}

/**
 * @brief Prints a dataset performance measurement error to `stderr`.
 * @param step Step of dataset loading the error happened in.
 */
void __performance_metrics_print_dataset_measurement_error(
    performance_metrics_dataset_step_t step) {

    const char *where;
    switch (step) {
        case PERFORMANCE_METRICS_DATASET_STEP_USERS:
            where = "users.csv";
            break;
        case PERFORMANCE_METRICS_DATASET_STEP_FLIGHTS:
            where = "flights.csv";
            break;
        case PERFORMANCE_METRICS_DATASET_STEP_PASSENGERS:
            where = "passengers.csv";
            break;
        case PERFORMANCE_METRICS_DATASET_STEP_RESERVATIONS:
            where = "reservations.csv";
            break;
        default:
            where = "?.csv";
            break;
    };
    fprintf(stderr, "Failed to perform resource usage measurement in dataset! (%s)\n", where);
}

void performance_metrics_measure_dataset(performance_metrics_t             *metrics,
                                         performance_metrics_dataset_step_t step) {
    if (!metrics)
        return;

    /* Finish previous measurement (don't act on non-existant measurements) */
    if (metrics->current_dataset_step != PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED &&
        metrics->dataset_events[metrics->current_dataset_step]) {

        if (performance_event_stop_measuring(
                metrics->dataset_events[metrics->current_dataset_step]))
            __performance_metrics_print_dataset_measurement_error(metrics->current_dataset_step);
    }

    /* Start current measurement */
    if (step == PERFORMANCE_METRICS_DATASET_STEP_DONE)
        return;

    performance_event_t *const perf = performance_event_start_measuring();
    if (!perf)
        __performance_metrics_print_dataset_measurement_error(step);

    metrics->current_dataset_step = step;
    metrics->dataset_events[step] = perf;
}

void performance_metrics_start_measuring_query_statistics(performance_metrics_t *metrics,
                                                          size_t                 query_type) {
    if (!metrics)
        return;

    if (metrics->statistical_events[query_type - 1])
        performance_event_free(metrics->statistical_events[query_type - 1]);

    performance_event_t *const perf = performance_event_start_measuring();
    if (!perf)
        fprintf(stderr,
                "Failed to measure resource usage in query %zu's statistical data generation!\n",
                query_type);

    metrics->statistical_events[query_type - 1] = perf;
}

void performance_metrics_stop_measuring_query_statistics(performance_metrics_t *metrics,
                                                         size_t                 query_type) {
    if (!metrics)
        return;

    if (!metrics->statistical_events[query_type - 1] ||
        performance_event_stop_measuring(metrics->statistical_events[query_type - 1])) {

        fprintf(stderr,
                "Failed to measure resource usage in query %zu's statistical data generation!\n",
                query_type);
    }
}

void performance_metrics_start_measuring_query_execution(performance_metrics_t *metrics,
                                                         size_t                 query_type,
                                                         size_t                 line_in_file) {
    if (!metrics)
        return;

    performance_event_t *const perf = performance_event_start_measuring();
    if (!perf)
        fprintf(stderr,
                "Failed to measure resource usage in query %zu's (line %zu) execution!\n",
                query_type,
                line_in_file);

    g_hash_table_insert(metrics->query_events[query_type - 1],
                        GUINT_TO_POINTER(line_in_file),
                        perf);
}

void performance_metrics_stop_measuring_query_execution(performance_metrics_t *metrics,
                                                        size_t                 query_type,
                                                        size_t                 line_in_file) {
    if (!metrics)
        return;

    performance_event_t *const perf =
        g_hash_table_lookup(metrics->query_events[query_type - 1], GUINT_TO_POINTER(line_in_file));

    if (!perf || performance_event_stop_measuring(perf))
        fprintf(stderr,
                "Failed to measure resource usage in query %zu's (line %zu) execution!\n",
                query_type,
                line_in_file);
}

void performance_metrics_measure_whole_program(performance_metrics_t *metrics) {
    if (!metrics)
        return;

    struct rusage usage;
    const int     insuccess = getrusage(RUSAGE_SELF, &usage);
    if (insuccess) {
        fputs("Failed to measure resource usage of the whole program!\n", stderr);
        metrics->program_total_time = 0;
        metrics->program_total_mem  = 0;
    } else {
        metrics->program_total_time = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000000 +
                                      (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec);
        metrics->program_total_mem = usage.ru_maxrss;
    }
}

const performance_event_t *
    performance_metrics_get_dataset_measurement(const performance_metrics_t       *metrics,
                                                performance_metrics_dataset_step_t step) {
    return metrics->dataset_events[step];
}

const performance_event_t *
    performance_metrics_get_query_statistics_measurement(const performance_metrics_t *metrics,
                                                         size_t                       query_type) {
    return metrics->statistical_events[query_type - 1];
}

uint64_t performance_metrics_get_program_total_time(const performance_metrics_t *metrics) {
    return metrics->program_total_time;
}

size_t performance_metrics_get_program_total_mem(const performance_metrics_t *metrics) {
    return metrics->program_total_mem;
}

/**
 * @brief   Comparison function, for `qsort` of an array of `size_t`.
 * @details Auxiliary method for ::performance_metrics_get_query_execution_measurements.
 */
int __performance_metrics_size_compare_func(const void *a, const void *b) {
    return *(const size_t *) a - *(const size_t *) b;
}

size_t performance_metrics_get_query_execution_measurements(const performance_metrics_t *metrics,
                                                            size_t                       query_type,
                                                            size_t   **out_line_numbers,
                                                            uint64_t **out_times) {
    guint         length;
    size_t *const line_numbers_gmalloc =
        (size_t *) g_hash_table_get_keys_as_array(metrics->query_events[query_type - 1], &length);
    qsort(line_numbers_gmalloc, length, sizeof(size_t), __performance_metrics_size_compare_func);

    /* Make line_numbers a malloc (instead of g_malloc) allocated pointer */
    size_t *const line_numbers = malloc(sizeof(size_t) * length);
    memcpy(line_numbers, line_numbers_gmalloc, sizeof(size_t) * length);
    g_free(line_numbers_gmalloc);

    uint64_t *const times = malloc(length * sizeof(uint64_t));

    for (size_t i = 0; i < length; ++i) {
        const performance_event_t *const perf =
            g_hash_table_lookup(metrics->query_events[query_type - 1],
                                GUINT_TO_POINTER(line_numbers[i]));

        times[i] = performance_event_get_elapsed_time(perf);
    }

    *out_line_numbers = line_numbers;
    *out_times        = times;
    return length;
}

void performance_metrics_free(performance_metrics_t *metrics) {
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i)
        if (metrics->dataset_events[i])
            performance_event_free(metrics->dataset_events[i]);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        if (metrics->statistical_events[i])
            performance_event_free(metrics->statistical_events[i]);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        if (metrics->query_events[i]) /* If statement to ease cleanup after allocation failure */
            g_hash_table_unref(metrics->query_events[i]);

    free(metrics);
}
