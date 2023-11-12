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
 * @var query_file_parser_data_t::aux_query
 *     @brief Single query instance, overwritten multiple times to reduce the number of allocations.
 * @var query_file_parser_data_t::line_number
 *     @brief Number of the current line of the file.
 * @var query_file_parser_data_t::query_instance_list
 *     @brief List to add parsed queries to.
 * @var query_file_parser_data_t::query_type_list
 *     @brief List of known query types (numerical identifiers).
 */
typedef struct {
    GPtrArray             *aux_buffer;
    query_instance_t      *aux_query;
    size_t                 line_number;
    query_instance_list_t *query_instance_list;
    query_type_list_t     *query_type_list;
} query_file_parser_data_t;

/**
 * @brief   Parses a line (containing a query) in a query file.
 * @details Auxiliary method for ::query_file_parser_parse.
 *
 * @param user_data A pointer to a ::query_file_parser_data_t.
 * @param line      Query to be parsed.
 */
int __query_file_parser_parse_query_callback(void *user_data, char *line) {
    query_file_parser_data_t *parser_data = (query_file_parser_data_t *) user_data;

    int retval = query_parser_parse_string(parser_data->aux_query,
                                           line,
                                           parser_data->query_type_list,
                                           parser_data->aux_buffer);
    if (retval)
        return 0; /* Ignore parsing failures */

    query_instance_set_number_in_file(parser_data->aux_query, parser_data->line_number);
    query_instance_list_add(parser_data->query_instance_list, parser_data->aux_query);

    parser_data->line_number++;
    return 0;
}

query_instance_list_t *query_file_parser_parse(FILE *input, query_type_list_t *query_type_list) {
    query_instance_list_t *list = query_instance_list_create();
    if (!list)
        return NULL;

    query_instance_t *aux_query = query_instance_create();
    if (!aux_query) {
        query_instance_list_free(list, query_type_list);
        return NULL;
    }

    query_file_parser_data_t parser_data = {.aux_buffer          = g_ptr_array_new(),
                                            .aux_query           = aux_query,
                                            .line_number         = 1,
                                            .query_instance_list = list,
                                            .query_type_list     = query_type_list};

    stream_tokenize(input, '\n', __query_file_parser_parse_query_callback, &parser_data);

    g_ptr_array_free(parser_data.aux_buffer, TRUE);
    free(aux_query); /* Don't use query_instance_free, not to free internal data */
    return list;
}
