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
 */
int query_parser_parse_string_const(query_instance_t  *output,
                                    const char        *input,
                                    query_type_list_t *query_type_list,
                                    GPtrArray         *aux);

#endif
