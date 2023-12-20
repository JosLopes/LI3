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
 * @file  performance_event.c
 * @brief Implementation of methods in include/performance/performance_event.h
 *
 * #### Examples
 * See [the header file's documentation](@ref performance_event_example).
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#include "performance/performance_event.h"
#include "utils/int_utils.h"
#include "utils/stream_utils.h"

/**
 * @struct performance_event
 * @brief  Information about elapsed time and used memory while running a task.
 *
 * @var performance_event::elapsed_time
 *     @brief   Time spent in microseconds. Includes both system and user time.
 *     @details If ::performance_event_end_measuring has been called, this is the time spent running
 *              the task. Otherwise, it's the time running the program until the task.
 * @var performance_event::used_memory
 *     @brief   Memory spent (in KiB).
 *     @details If ::performance_event_end_measuring has been called, this is the difference between
 *              memory in the beginning and in the end of the task, clamped to zero. Otherwise,
 *              it's the memory used before starting to run the task.
 */
struct performance_event {
    uint64_t elapsed_time;
    size_t   used_memory;
};

/**
 * @brief Adds time spent by the program (@p user) and the kernel (@p system).
 *
 * @param user   User time, this is, time spent running program instructions.
 * @param system System time, this is, time spent in kernel code.
 *
 * @return The sum of @p user and @p system, in microseconds.
 */
uint64_t __performance_event_add_user_system_time(struct timeval user, struct timeval system) {
    return (uint64_t) (user.tv_sec + system.tv_sec) * 1000000 + user.tv_usec + system.tv_usec;
}

/**
 * @brief Parses a line from `/proc/self/status`, looking for `VmSize`.
 *
 * @param user_data A pointer to a `size_t`, where to write the ammount of used memory to.
 * @param line      Line of `/proc/self/status` to be parsed.
 *
 * @retval 0 `VmSize` not found on this line. Continue parsing.
 * @retval 1 `VmSize` found on this line. Stop parsing.
 */
int __performance_event_get_memory_usage_parse_line(void *user_data, char *line) {
    if (strncmp(line, "VmSize:", strlen("VmSize:")) == 0) {
        /* Find number */
        char *number = line;
        for (; *number && !isdigit(*number); ++number)
            ;

        /* Set end of number */
        char *end = number;
        for (; *end && *end != ' '; ++end)
            ;
        *end = '\0';

        uint64_t used;
        if (!int_utils_parse_positive(&used, number))
            *((size_t *) user_data) = used;

        return 1; /* Stop tokenization */
    }

    return 0;
}

/**
 * @brief   Parses `/proc/self/status` to get memory usage information
 * @details Exclusive to Linux. Wouldn't be needed if `getrusage` weren't broken (Linux does not
 *          expose current memory usage, while other BSDs do).
 *
 * @param output Where to output the memory usage (in KiB), only on success.
 *
 * @retval 0 Success
 * @retval 1 Failure
 */
int __performance_event_get_memory_usage(size_t *output) {
    FILE *status = fopen("/proc/self/status", "r");
    if (!status)
        return 1;

    if (stream_tokenize(status, '\n', __performance_event_get_memory_usage_parse_line, output) !=
        1) {
        fclose(status);
        return 1;
    }

    fclose(status);
    return 0;
}

performance_event_t *performance_event_start_measuring(void) {
    performance_event_t *perf = malloc(sizeof(performance_event_t));
    if (!perf)
        return NULL;

    if (__performance_event_get_memory_usage(&perf->used_memory)) {
        free(perf);
        return NULL;
    }

    struct rusage usage;
    int           insuccess = getrusage(RUSAGE_SELF, &usage);
    if (insuccess) {
        free(perf);
        return NULL;
    }
    perf->elapsed_time = __performance_event_add_user_system_time(usage.ru_utime, usage.ru_stime);

    return perf;
}

performance_event_t *performance_event_clone(const performance_event_t *perf) {
    performance_event_t *ret = malloc(sizeof(performance_event_t));
    if (!ret)
        return NULL;

    memcpy(ret, perf, sizeof(performance_event_t));
    return ret;
}

int performance_event_stop_measuring(performance_event_t *perf) {
    struct rusage usage;
    int           insuccess = getrusage(RUSAGE_SELF, &usage);
    if (insuccess) {
        perf->elapsed_time = 0;
        perf->used_memory  = 0;
        return 1;
    }

    uint64_t elapsed   = __performance_event_add_user_system_time(usage.ru_utime, usage.ru_stime);
    perf->elapsed_time = elapsed - perf->elapsed_time;

    size_t new_memory;
    if (__performance_event_get_memory_usage(&new_memory)) {
        perf->elapsed_time = 0;
        perf->used_memory  = 0;
        return 1;
    }

    if (perf->used_memory > new_memory)
        perf->used_memory = 0;
    else
        perf->used_memory = new_memory - perf->used_memory;

    return 0;
}

uint64_t performance_event_get_elapsed_time(const performance_event_t *perf) {
    return perf->elapsed_time;
}

size_t performance_event_get_used_memory(const performance_event_t *perf) {
    return perf->used_memory;
}

void performance_event_free(performance_event_t *perf) {
    free(perf);
}
