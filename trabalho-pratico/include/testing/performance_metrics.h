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
 * @file  performance_metrics.h
 * @brief Performance information about the whole application.
 *
 * @anchor performance_metrics_example
 * ### Example
 *
 * See test.c for usage examples. Also, see
 * [performance_metrics_output](@ref performance_metrics_output.h) to display the data in a
 * ::performance_metrics_t.
 *
 * Other methods (such as ::performance_metrics_measure_dataset) are tightly related to the
 * inner workings of the application, so they are only to be used by the specific modules that
 * constitute the different functionalities of the application.
 */

#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include "testing/performance_event.h"

/** @brief Step of loading a dataset, whose performance must be measured. */
typedef enum {
    PERFORMANCE_METRICS_DATASET_STEP_USERS,        /**< @brief Loading `users.csv`.         */
    PERFORMANCE_METRICS_DATASET_STEP_FLIGHTS,      /**< @brief Loading `flights.csv`.       */
    PERFORMANCE_METRICS_DATASET_STEP_PASSENGERS,   /**< @brief Loading `passengers.csv`.    */
    PERFORMANCE_METRICS_DATASET_STEP_RESERVATIONS, /**< @brief Loading `reservations.csv`.  */
    PERFORMANCE_METRICS_DATASET_STEP_DONE,         /**< @brief Done loading the dataset.    */
    PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED,  /**< @brief Not yet loading the dataset. */
} performance_metrics_dataset_step_t;

/** @brief Performance information about different parts of the application. */
typedef struct performance_metrics performance_metrics_t;

/**
 * @brief  Initializes a set of performance metrics for the whole application
 * @return A pointer to a new ::performance_metrics_t, that must be deleted using
 *         ::performance_metrics_free. `NULL` is possible on failure.
 *
 * #### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */
performance_metrics_t *performance_metrics_create(void);

/**
 * @brief  Creates a deep clone of a ::performance_metrics_t.
 * @param  metrics Performance metrics to be cloned.
 * @return A pointer to a new ::performance_metrics_t, that must be deleted using
 *         ::performance_metrics_free. `NULL` is possible on failure.
 */
performance_metrics_t *performance_metrics_clone(const performance_metrics_t *metrics);

/**
 * @brief   Measures a performance event for a step of loading a dataset.
 * @details Measuring failures are reported to `stderr`.
 *
 *  When a new step of the dataset starts being loaded, it is considered that the previous one is
 *  done. When done parsing the dataset, call this method with the value
 *  ::PERFORMANCE_METRICS_DATASET_STEP_DONE for the parameter @p step.
 *
 * @param metrics Performance metrics to be modified. Can be `NULL`, for no performance profiling.
 * @param step    Step of the dataset that is starting to be loaded.
 */
void performance_metrics_measure_dataset(performance_metrics_t             *metrics,
                                         performance_metrics_dataset_step_t step);

/**
 * @brief   Starts measuring a performance event for the generation of statistical data for a query.
 * @details When the query's data is generated, call
 *          ::performance_metrics_stop_measuring_query_statistics. Measuring failures are reported
 *          to `stderr`.
 *
 * @param metrics    Performance metrics to be modified. Can be `NULL`, for no performance
 *                   profiling.
 * @param query_type Query type for which statistical data is starting to be generated.
 */
void performance_metrics_start_measuring_query_statistics(performance_metrics_t *metrics,
                                                          size_t                 query_type);

/**
 * @brief   Finishes measuring a performance event for the generation of statistical data for a
 *          query.
 * @details Measuring failures are reported to `stderr`.
 *
 * @param metrics    Performance metrics to be modified. Can be `NULL`, for no performance
 *                   profiling.
 * @param query_type Query type for which statistical data is done being generated.
 */
void performance_metrics_stop_measuring_query_statistics(performance_metrics_t *metrics,
                                                         size_t                 query_type);

/**
 * @brief   Starts measuring a performance event for the execution of a query.
 * @details When the query is done executing, call
 *          ::performance_metrics_stop_measuring_query_statistics. Measuring failures are reported
 *          to `stderr`.
 *
 * @param metrics      Performance metrics to be modified. Can be `NULL`, for no performance
 *                     profiling.
 * @param query_type   Query type for which statistical data is being generated.
 * @param line_in_file Line of the query in the batch mode's input file.
 */
void performance_metrics_start_measuring_query_execution(performance_metrics_t *metrics,
                                                         size_t                 query_type,
                                                         size_t                 line_in_file);

/**
 * @brief   Finishes measuring a performance event for the execution of a query.
 * @details Measuring failures are reported to `stderr`.
 *
 * @param metrics      Performance metrics to be modified. Can be `NULL`, for no performance
 *                     profiling.
 * @param query_type   Query type for which statistical data is being generated.
 * @param line_in_file Line of the query in the batch mode's input file.
 */
void performance_metrics_stop_measuring_query_execution(performance_metrics_t *metrics,
                                                        size_t                 query_type,
                                                        size_t                 line_in_file);

/**
 * @brief   Measures execution time and peak memory usage of the whole program.
 * @details Must be called after the program is done executing and before @p metrics are displayed.
 *          Measuring failures are reported to `stderr`.
 *
 * @param metrics Performance metrics to be modified. Can be `NULL`, for no performance profiling.
 *
 * #### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */
void performance_metrics_measure_whole_program(performance_metrics_t *metrics);

/**
 * @brief Gets a measurement of dataset loading performance from a ::performance_metrics_t.
 *
 * @param metrics Performance metrics to get dataset loading performance information from.
 * @param step    Phase of dataset loading to be considered. Musn't be
 *                ::PERFORMANCE_METRICS_DATASET_STEP_DONE or
 *                ::PERFORMANCE_METRICS_DATASET_STEP_NOT_STARTED.
 *
 * @return Performance information about loading a part of a dataset, or `NULL` if that hasn't yet
 *         been measured / failed to be measured.
 */
const performance_event_t *
    performance_metrics_get_dataset_measurement(const performance_metrics_t       *metrics,
                                                performance_metrics_dataset_step_t step);

/**
 * @brief Gets a measurement of query statistical data generation performance from a
 *        ::performance_metrics_t.
 *
 * @param metrics    Performance metrics to get query statistical data generation performance
 *                   information from.
 * @param query_type Query type whose statistical data generation has been profiled.
 *
 * @return Performance information about generating statistical data for a query, or `NULL` if that
 *         hasn't yet been measured / failed to be measured.
 */
const performance_event_t *
    performance_metrics_get_query_statistics_measurement(const performance_metrics_t *metrics,
                                                         size_t                       query_type);

/**
 * @brief   Gets the time measurements from executions of all queries of type @p query_type in a
 *          ::performance_metrics_t.
 * @details On success, `malloc`-allocated pointers will be written to @p out_line_numbers and
 *          @p out_times, which subsequently need to be `free`d. Memory information isn't exposed
 *          because query execution isn't supposed to allocate measurable amounts of memory. It's
 *          accepted for query statistical generation to do so, but not execution.
 *
 * @param metrics          Performance metrics to get performance information from.
 * @param query_type       Query type whose executions have been profiled.
 * @param out_line_numbers Where to output the lines where the queries were (will be ordered).
 * @param out_times        Where to output the times that it took to execute queries (in
 *                         microseconds).
 *
 * @return The number of elements in @p out_line_numbers and @p out_times.
 */
size_t performance_metrics_get_query_execution_measurements(const performance_metrics_t *metrics,
                                                            size_t                       query_type,
                                                            size_t   **out_line_numbers,
                                                            uint64_t **out_times);

/**
 * @brief   Gets the time it took to run the whole program from a ::performance_metrics_t.
 * @details Must be called after ::performance_metrics_measure_whole_program.
 *
 * @param  metrics Performance metrics to get performance information from.
 *
 * @return The time (in microseconds) it took to run the whole program. `0` can be returned if that
 *         hasn't been measured or if that measurement failed.
 */
uint64_t performance_metrics_get_program_total_time(const performance_metrics_t *metrics);

/**
 * @brief   Gets the peak memory usage during the program's execution from a
 *          ::performance_metrics_t.
 * @details Must be called after ::performance_metrics_measure_whole_program.
 *
 * @param  metrics Performance metrics to get performance information from.
 *
 * @return The peak memory usage in KiB (kibikytes). `0` can be returned if that hasn't been
 *         measured or if that measurement failed.
 */
size_t performance_metrics_get_program_total_mem(const performance_metrics_t *metrics);

/**
 * @brief Frees memory in a ::performance_metrics_t.
 * @param metrics Performance metrics to be `free`'d.
 *
 * #### Example
 * See [the header file's documentation](@ref performance_metrics_example).
 */
void performance_metrics_free(performance_metrics_t *metrics);

#endif
