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
 * @file  query_file_parser.h
 * @brief A parser for a file containing a query in each line.
 *
 * @anchor query_file_parser_examples
 * ### Examples
 * TODO - query file parser examples (when the rest of the query system, including dispatcher, is
 *        ready)
 */

#ifndef QUERY_FILE_PARSER_H
#define QUERY_FILE_PARSER_H

#include <stdio.h>

#include "queries/query_instance_list.h"

/**
 * @brief Parses a file containg a query in each line.
 *
 * @param input           Input file stream to be read.
 * @param query_type_list List of known queries.
 *
 * @return A pointer to a ::query_instance_list_t, that must later be `free`'d by
 *         ::query_instance_list_free, or `NULL` on failure.
 */
query_instance_list_t *query_file_parser_parse(FILE *input, query_type_list_t *query_type_list);

#endif
