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
 * @file  performance_event.h
 * @brief Information about elapsed time and used memory while running a task.
 *
 * @anchor performance_event_example
 * ### Example
 *
 * ```c
 * performance_event_t *perf = performance_event_start_measuring();
 * if (!perf)
 *     return 1;
 *
 * char *mem_test = malloc(1 << 24); // Allocate 16 MiB
 * strcpy(mem_test, "Hello, world");
 * puts(mem_test);
 *
 * if (performance_event_stop_measuring(perf)) {
 *     performance_event_free(perf);
 *     free(mem_test);
 *     return 1;
 * }
 *
 * printf("Time: %" PRIu64 " us\nMemory: %zu KiB\n",
 *        performance_event_get_elapsed_time(perf),
 *        performance_event_get_used_memory(perf));
 *
 * free(mem_test);
 * performance_event_free(perf);
 * return 0;
 * ```
 *
 * Here's the output on my system:
 *
 * ```text
 * Hello, world
 * Time: 87 us
 * Memory: 16388 KiB
 * ```
 */

#ifndef PERFORMANCE_EVENT_H
#define PERFORMANCE_EVENT_H

#include <inttypes.h>
#include <stddef.h>

/** @brief Information about elapsed time and used memory while running a task. */
typedef struct performance_event performance_event_t;

/**
 * @brief  Starts collecting data to measure the performance of a task.
 * @return A new performance event, that must be deleted with ::performance_event_free, or `NULL` in
 *         case of failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */
performance_event_t *performance_event_start_measuring(void);

/**
 * @brief Creates a deep clone of a performance event.
 * @param perf Performance event to be cloned.
 * @return A pointer to a new ::performance_event_t, that must be deleted using
 *         ::performance_event_free. `NULL` is possible on failure.
 */
performance_event_t *performance_event_clone(const performance_event_t *perf);

/**
 * @brief Terminates data collection to measure the performance of a task.
 * @param perf Event to be modified.
 *
 * @retval 0 Success.
 * @retval 1 Measurement failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */
int performance_event_stop_measuring(performance_event_t *perf);

/**
 * @brief   Gets the time that it took to run a task, in microseconds.
 * @details ::performance_event_end_measuring must have been called before this method.
 *
 * @param perf Performance event to get the elapsed time from.
 *
 * @return The time that it took to run a task, in microseconds.
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */
uint64_t performance_event_get_elapsed_time(const performance_event_t *perf);

/**
 * @brief   Gets the memory usage, in kibibytes, that a task added to the program.
 * @details ::performance_event_end_measuring must have been called before this method.
 *
 * @param perf Performance event to get the memory from.
 *
 * @return The difference between memory usage in the beginning and in the end of a task, clamped to
 *         zero (in case the task `free`'d more memory than allocated). No peak memory usage is
 *         considered.
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */
size_t performance_event_get_used_memory(const performance_event_t *perf);

/**
 * @brief Frees memory allocated in ::performance_event_start_measuring.
 * @param perf Event to have its memory `free`'d.
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */
void performance_event_free(performance_event_t *perf);

#endif
