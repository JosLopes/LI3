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
 * @brief Implementation of methods in include/performance/performance_metrics.h
 *
 * ### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */

#include <stdio.h>
#include <stdlib.h>

#include "performance/performance_event.h"
#include "performance/performance_metrics.h"
#include "queries/query_type_list.h"

/**
 * @struct performance_metrics
 * @brief  Information about performance for dataset loading and query execution.
 *
 * @var performance_metrics::current_dataset_step
 *     @brief Current part of the dataset being loaded.
 * @var performance_metrics::dataset_events
 *     @brief Performance information about dataset loading.
 * @var performance_metrics::statistical_events
 *     @brief Performance information about query statistical data collection.
 */
struct performance_metrics {
    performance_metrics_dataset_step_t current_dataset_step;
    performance_event_t               *dataset_events[PERFORMANCE_METRICS_DATASET_STEP_DONE];
    performance_event_t               *statistical_events[QUERY_TYPE_LIST_COUNT];
};

performance_metrics_t *performance_metrics_create(void) {
    performance_metrics_t *ret = malloc(sizeof(performance_metrics_t));
    if (!ret)
        return NULL;

    ret->current_dataset_step = PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED;
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i)
        ret->dataset_events[i] = NULL;

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        ret->statistical_events[i] = NULL;

    return ret;
}

performance_metrics_t *performance_metrics_clone(const performance_metrics_t *metrics) {
    performance_metrics_t *ret = malloc(sizeof(performance_metrics_t));
    if (!ret)
        return NULL;

    ret->current_dataset_step = metrics->current_dataset_step;
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i)
        ret->dataset_events[i] = performance_event_clone(metrics->dataset_events[i]);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        ret->statistical_events[i] = performance_event_clone(metrics->statistical_events[i]);

    return ret;
}

/**
 * @brief Prints a dataset performance measurement errpr to `stderr`.
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
            where = "unknown location";
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

        int insuccess = performance_event_stop_measuring(
            metrics->dataset_events[metrics->current_dataset_step]);
        if (insuccess)
            __performance_metrics_print_dataset_measurement_error(metrics->current_dataset_step);
    }

    /* Start current measurement */

    if (step == PERFORMANCE_METRICS_DATASET_STEP_DONE)
        return;

    performance_event_t *perf = performance_event_start_measuring();
    if (!perf)
        __performance_metrics_print_dataset_measurement_error(step);

    metrics->current_dataset_step = step;
    metrics->dataset_events[step] = perf;
}

void performance_metrics_start_measuring_query_statistics(performance_metrics_t *metrics,
                                                          size_t                 query_type) {
    if (!metrics)
        return;

    performance_event_t *perf = performance_event_start_measuring();
    if (!perf)
        fprintf(stderr,
                "Failed to measure resource usage in query %zu's statistical data!\n",
                query_type);

    metrics->statistical_events[query_type] = perf;
}

void performance_metrics_stop_measuring_query_statistics(performance_metrics_t *metrics,
                                                         size_t                 query_type) {
    if (!metrics)
        return;

    if (!metrics->statistical_events[query_type] ||
        performance_event_stop_measuring(metrics->statistical_events[query_type])) {

        fprintf(stderr,
                "Failed to measure resource usage in query %zu's statistical data!\n",
                query_type);
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
    return metrics->statistical_events[query_type];
}

void performance_metrics_free(performance_metrics_t *metrics) {
    for (size_t i = 0; i < PERFORMANCE_METRICS_DATASET_STEP_DONE; ++i)
        if (metrics->dataset_events[i])
            performance_event_free(metrics->dataset_events[i]);

    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        if (metrics->statistical_events[i])
            performance_event_free(metrics->statistical_events[i]);

    free(metrics);
}
