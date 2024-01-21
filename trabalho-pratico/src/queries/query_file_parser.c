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
 * @file  query_file_parser.c
 * @brief Implementation of methods in include/queries/query_file_parser.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_file_parser_examples).
 */

#include <glib.h>

#include "queries/query_file_parser.h"
#include "queries/query_parser.h"
#include "utils/stream_utils.h"

/**
 * @struct query_file_parser_data_t
 * @brief  State of a parser of a file of queries.
 *
 * @var query_file_parser_data_t::aux_buffer
 *     @brief Auxiliary array passed to ::query_parser_parse_string, to reduce the number of
 *            allocations.
 * @var query_file_parser_data_t::line_number
 *     @brief Number of the current line of the file.
 * @var query_file_parser_data_t::query_instance_list
 *     @brief List to add parsed queries to.
 */
typedef struct {
    GPtrArray *const             aux_buffer;
    size_t                       line_number;
    query_instance_list_t *const query_instance_list;
} query_file_parser_data_t;

/**
 * @brief   Parses a line (containing a query) in a query file.
 * @details Auxiliary method for ::query_file_parser_parse.
 *
 * @param user_data A pointer to a ::query_file_parser_data_t.
 * @param line      Query to be parsed.
 *
 * @retval 0 Success (parsing failures may occur).
 * @retval 1 Allocation failure.
 */
int __query_file_parser_parse_query_callback(void *user_data, char *line) {
    query_file_parser_data_t *const parser_data = user_data;

    query_instance_t *const aux_query = query_instance_create();
    if (!aux_query)
        return 1;

    const int retval = query_parser_parse_string(aux_query, line, parser_data->aux_buffer);
    if (retval) {
        query_instance_free(aux_query);
        parser_data->line_number++;
        return 0; /* Ignore parsing failures */
    }

    query_instance_set_line_in_file(aux_query, parser_data->line_number);
    if (query_instance_list_add(parser_data->query_instance_list, aux_query)) {
        query_instance_free(aux_query);
        return 1; /* Allocation failure */
    }
    query_instance_free(aux_query);

    parser_data->line_number++;
    return 0;
}

query_instance_list_t *query_file_parser_parse(FILE *input) {
    query_instance_list_t *const list = query_instance_list_create();
    if (!list)
        return NULL;

    query_file_parser_data_t parser_data = {.aux_buffer          = g_ptr_array_new(),
                                            .line_number         = 1,
                                            .query_instance_list = list};

    if (stream_tokenize(input, '\n', __query_file_parser_parse_query_callback, &parser_data)) {
        g_ptr_array_unref(parser_data.aux_buffer);
        query_instance_list_free(list);
        return NULL;
    }

    g_ptr_array_unref(parser_data.aux_buffer);
    return list;
}
