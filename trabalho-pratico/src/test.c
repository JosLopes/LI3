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

#include "queries/query_parser.h"

/**
 * @brief The entry point to the test program.
 * @details Tests for query parsing.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    const char *queries[5] = {"88F Hello world",
                              "1 \"Hello, world\" test      123 \"last ends in quotes\"",
                              "1F \"multiple   spaces\"",
                              "3 \"unknown query\" \"number\"",
                              "3F \"unknown formatted\" \"query\""};

    query_type_list_t *query_list = query_type_list_create();
    if (!query_list) {
        fprintf(stderr, "Failed to allocate query definitions!\n");
        return 1;
    }

    GPtrArray *aux = g_ptr_array_new();

    for (size_t i = 0; i < 5; ++i) {
        query_instance_t *query = query_instance_create();

        int result = query_parser_parse_string_const(query, queries[i], query_list, aux);
        if (result)
            fprintf(stderr, "Failed to parse query: %s\n", queries[i]);
        else if (query_instance_get_formatted(query))
            printf("Query's output must be formatted!\n");
        else
            printf("Query's output should not be formatted!\n");

        query_instance_free(query, query_list);
    }

    query_type_list_free(query_list);
    g_ptr_array_free(aux, TRUE);
    return 0;
}
