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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/date.h"

void *pthread_callback(void * arg) { /* Parse the same date 2^15 times */
    (void) arg;

    char parse[256] = "2023/12/02";
    date_t date;
    for (int i = 0; i < (1 << 15); ++i)
        date_from_string(&date, parse);

    return NULL;
}

/**
 * @brief The entry point to the test program.
 * @details Tests for date grammar thread safety.
 * @retval 0 Success
 * @retval 1 Failure
 */
int main(void) {
    pthread_t threads[8];

    for (int i = 0; i < 8; ++i)
        pthread_create(threads + i, NULL, pthread_callback, NULL);
    for (int i = 0; i < 8; ++i)
        pthread_join(threads[i], NULL);

    return 0;
}
