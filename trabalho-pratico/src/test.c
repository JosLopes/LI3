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
 * @file test.c
 * @brief Contains the entry point to the test program.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "performance/performance_event.h"

/**
 * @brief The entry point to the test program.
 * @details Tests for resource usage measurement.
 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    performance_event_t *perf = performance_event_start_measuring();
    if (!perf)
        return 1;

    char *mem_test = malloc(1 << 24); /* Allocate 16 MiB */
    strcpy(mem_test, "Hello, world");
    puts(mem_test);

    if (performance_event_stop_measuring(perf)) {
        performance_event_free(perf);
        free(mem_test);
        return 1;
    }

    printf("Time: %" PRIu64 " us\nMemory: %zu KiB\n",
           performance_event_get_elapsed_time(perf),
           performance_event_get_used_memory(perf));

    free(mem_test);
    performance_event_free(perf);
    return 0;
}
