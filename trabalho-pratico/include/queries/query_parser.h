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
 * @file  query_parser.h
 * @brief Parser of a single query.
 *
 * @anchor query_parser_examples
 * ### Examples
 *
 * The following example uses ::query_parser_parse_string_const. ::query_parser_parse_string has
 * the same behavior for non-constant strings. Also, suppose that query 1's
 * ::query_type_parse_arguments_callback just prints the queries arguments to `stdout`, and there
 * are no other queries.
 *
 * ```c
 * int main(void) {
 *     const char *queries[4] = {"1  \"Hello, world\" test      123 \"last ends in quotes\"",
 *                               "1F \"multiple   spaces\"",
 *                               "3  \"unknown query\" \"number\"",
 *                               "3F \"unknown formatted\" \"query\""};
 *
 *     query_type_list_t *query_list = query_type_list_create();
 *     if (!query_list) {
 *         fprintf(stderr, "Failed to allocate query definitions!\n");
 *         return 1;
 *     }
 *
 *     GPtrArray *aux = g_ptr_array_new();
 *
 *     for (size_t i = 0; i < 4; ++i) {
 *         query_instance_t *query = query_instance_create();
 *
 *         int result = query_parser_parse_string_const(query, queries[i], query_list, aux);
 *         if (result)
 *             fprintf(stderr, "Failed to parse query: %s\n", queries[i]);
 *         else if (query_instance_get_formatted(query))
 *             printf("Query's output must be formatted!\n\n");
 *         else
 *             printf("Query's output should not be formatted!\n\n");
 *
 *         query_instance_free(query, query_list);
 *     }
 *
 *     query_type_list_free(query_list);
 *     g_ptr_array_free(aux, TRUE);
 *     return 0;
 * }
 * ```
 *
 * The expected output, given this fictitious query, should be the following:
 *
 * ```text
 * Q1 arguments: [ "Hello, world", "test", "123", "last ends in quotes" ]
 * Query's output should not be formatted!
 *
 * Q1 arguments: [ "multiple   spaces" ]
 * Query's output must be formatted!
 *
 * Failed to parse query: 3  "unknown query" "number"
 * Failed to parse query: 3F "unknown formatted" "query"
 * ```
 *
 * Like in query_tokenizer.h, you can have arguments inside or outside quotes, and multiple
 * consecutive spaces are allowed both in quotes (kept) or outside quotes (discarded).
 *
 * It should be noted that, because we're parsing multiple queries, we used an external `GPtrArray`
 * that we provided to ::query_parser_parse_string_const. This way we avoid many allocations. For
 * parsing a single query, you can pass `NULL` to the mentioned method's `aux` paramater, and it'll
 * automatically allocate and de-allocate a new array.
 */

#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <glib.h>

#include "queries/query_instance.h"

/**
 * @brief Parses a **MODIFIABLE** string containing a query.
 *
 * @param output          Where the parsed query is placed. This will be modified on failure too.
 * @param input           String to parse, that will be modified in the parsing process.
 * @param query_type_list List of available queries.
 * @param aux             Auxiliary `GPtrArray`, that can be provided to be modified and avoid
 *                        memory allocations. If `NULL`, a new array will be instantiated.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 *
 * ### Examples
 * See [the header file's documentation](@ref query_parser_examples).
 */
int query_parser_parse_string(query_instance_t  *output,
                              char              *input,
                              query_type_list_t *query_type_list,
                              GPtrArray         *aux);

/**
 * @brief   Parses a string containing a query.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param output          Where the parsed query is placed. This will be modified on failure too.
 * @param input           String to parse.
 * @param query_type_list List of available queries.
 * @param aux             Auxiliary `GPtrArray`, that can be provided to be modified and avoid
 *                        memory allocations. If `NULL`, a new array will be instantiated.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing or allocation failure.
 *
 * ### Examples
 * See [the header file's documentation](@ref query_parser_examples).
 */
int query_parser_parse_string_const(query_instance_t  *output,
                                    const char        *input,
                                    query_type_list_t *query_type_list,
                                    GPtrArray         *aux);

#endif
