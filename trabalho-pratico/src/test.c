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

#include "utils/string_utils.h"

/**
 * Example data to be tokenized and parsed.
 */
#define STUDENT_HEIGHTS "160,170,182,165"

/**
 * @brief A @ref tokenize_iter_callback_t to sum heights and count students.
 * @param user_data An array with two integers, the accumulated height and the number of students,
 *                  respectively.
 * @param token Token read by @ref string_const_tokenize
 */
int iter(void *user_data, char *token) {
    int *sum_count = (int *) user_data;

    int height = atoi(token);
    if (height <= 0) {
        fprintf(stderr, "Invalid height: \"%s\"\n", token);
        return 1;
    } else {
        sum_count[0] += height; /* sum += height; */
        sum_count[1]++;         /* count++; */
    }

    return 0;
}

/**
 * @brief The entry point to the test program.
 * @details Calculates average height of a list of students.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    int sum_count[2] = {0, 0};

    if (string_const_tokenize(STUDENT_HEIGHTS, ',', iter, sum_count)) {
        return 1;
    } else {
        printf("Average height is: %.2fcm\n", (double) sum_count[0] / sum_count[1]);
        return 0;
    }
}
