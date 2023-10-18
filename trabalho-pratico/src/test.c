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
#include <string.h>

#include "utils/stream_utils.h"

int iter(void *user_data, char *token) {
    int *sum_count = (int *) user_data;

    int height = atoi(token);
    if (height <= 0) {
        fprintf(stderr, "Invalid height: \"%s\"\n", token);
        return 1;
    } else {
        sum_count[0] += height;
        sum_count[1]++;
    }
    
    return 0;
}

int main(void) {
    FILE *fp;
    fp = fopen("files/testfile", "r");

    int sum_count[2] = {0, 0};

    if (stream_tokenize(fp, ';', iter, sum_count))
    {
        return 1;
    } else {
        printf("Average height is: %.2fcm\n", (double) sum_count[0] / sum_count[1]);
        return 0;
    }
}

// TODO: Documentar o teste realizado
