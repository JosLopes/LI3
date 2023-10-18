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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <stdio.h>
#include <stdlib.h>

#include "utils/daytime.h"

/**
 * @brief The entry point to the test program.
 * @details Test for hours.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    const char *times[11] = {
        "00:44:24", /* Date during a sleepless programming session */

        "0:12:45",  /* One digit missing */
        "00:-1:12", /* Invalid digit */

        "00:00:00", /* Valid time */
        "23:59:59", /* Valid time */
        "24:59:59", /* Out of range hour */
        "23:60:59", /* Out of range minute */
        "23:59:60", /* Out of range second */

        "00:00:00:00", /* Too many data points */
        "00:00",       /* Too few data points */
        "",            /* What? */
    };

    for (int i = 0; i < 11; ++i) {
        daytime_t time;
        int       success = daytime_from_string_const(&time, times[i]);

        if (success) {
            fprintf(stderr, "Failed to parse time \"%s\".\n", times[i]);
        } else {
            char str[DAYTIME_SPRINTF_MIN_BUFFER_SIZE];
            daytime_sprintf(str, time);

            printf("%s was parsed successfully.\n", str);
        }
    }

    return 0;
}
