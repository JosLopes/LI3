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
 * @brief Contains the entry point to the program.
 */

#include <stdio.h>
#include <stdlib.h>

#include "queries/query_tokenizer.h"

int query_token_callback(void *user_data, char *token) {
    (void) user_data;
    printf("%s\n", token);
    return 0;
}

/**
 * @brief The entry point to the test program.
 * @details Tests for query parsing.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    const char *queries[4] = {"88F Hello world",
                              "1 \"Hello, world\"",
                              "2 \"multiple   spaces\"",
                              "3 \"multiple quotes\" \"in this query\""};

    for (size_t i = 0; i < 4; ++i) {
        int result = query_tokenizer_tokenize_const(queries[i], query_token_callback, NULL);
        if (result)
            fprintf(stderr, "Failed to parse query: %s\n", queries[i]);
        else
            putchar('\n');
    }

    return 0;
}
