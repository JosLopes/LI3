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
 * @file  query_parser.c
 * @brief Implementation of methods in include/queries/query_parser.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_parser_examples).
 */

#include <string.h>

#include "queries/query_parser.h"
#include "queries/query_tokenizer.h"
#include "utils/int_utils.h"

/**
 * @brief query_parser_data
 * @brief State of a query parser.
 *
 * @var query_parser_data_t::query_type_list
 *     @brief List of supported queries.
 * @var query_parser_data_t::output
 *     @brief Current query being parsed.
 * @var query_parser_data_t::args
 *     @brief Array to accumulate query arguments.
 * @var query_parser_data_t::first_token_parsed
 *     @brief Whether the first token (contains query number) has been parsed.
 * @var query_parser_data_t::last_terminator
 *     @brief Where null terminator (``'\0'``) should be placed, so that the last token ends. `NULL`
 *            if no token has yet been parsed.
 */
typedef struct {
    query_type_list_t *query_type_list;
    query_instance_t  *output;
    GPtrArray         *args;
    int                first_token_parsed;
    char              *last_terminator;
} query_parser_data_t;

int __query_parser_tokenize_callback(void *user_data, char *token) {
    query_parser_data_t *parser = (query_parser_data_t *) user_data;

    if (!parser->first_token_parsed) { /* First argument: query number */

        /* Check if query is formatted */
        size_t token_len = strlen(token);
        if (token[token_len - 1] == 'F') {
            query_instance_set_formatted(parser->output, 1);
            token[token_len - 1] = '\0';
        } else {
            query_instance_set_formatted(parser->output, 0);
        }

        /* Parse number of query */
        uint64_t query_type;
        int      retval = int_utils_parse_positive(&query_type, token);
        if (retval || !query_type_list_get_by_index(parser->query_type_list, (size_t) query_type)) {
            if (token[token_len - 1] == '\0') { /* Restore previous string form */
                token[token_len - 1] = 'F';
            }

            return 1;
        }
        query_instance_set_type(parser->output, (size_t) query_type);

        if (token[token_len - 1] == '\0') { /* Restore previous string form */
            token[token_len - 1] = 'F';
        }
        parser->first_token_parsed = 1;
    } else {
        if (parser->last_terminator)
            *parser->last_terminator = '\0';
        parser->last_terminator = token + strlen(token);

        g_ptr_array_add(parser->args, token);
    }

    return 0;
}

int query_parser_parse_string(query_instance_t  *output,
                              char              *input,
                              query_type_list_t *query_type_list,
                              GPtrArray         *aux) {

    query_parser_data_t parser_data = {.query_type_list    = query_type_list,
                                       .output             = output,
                                       .first_token_parsed = 0,
                                       .last_terminator    = NULL};
    if (aux)
        parser_data.args = aux;
    else
        parser_data.args = g_ptr_array_new();
    parser_data.args->len = 0;

    /* Query type parsing */
    int retval = query_tokenizer_tokenize(input, __query_parser_tokenize_callback, &parser_data);
    if (retval) {
        if (!aux)
            g_ptr_array_free(parser_data.args, TRUE);
        return 1;
    }

    if (parser_data.last_terminator)
        *parser_data.last_terminator = '\0';

    /* Argument parsing */
    query_type_t *query_type =
        query_type_list_get_by_index(query_type_list, query_instance_get_type(output));
    void *argument_data = query_type_get_parse_arguments_callback(
        query_type)((char **) parser_data.args->pdata, parser_data.args->len);

    if (!argument_data) { /* Argument parsing failure */
        if (!aux)
            g_ptr_array_free(parser_data.args, TRUE);
        return 1;
    }
    query_instance_set_argument_data(output, argument_data);

    if (!aux)
        g_ptr_array_free(parser_data.args, TRUE);
    return 0;
}

int query_parser_parse_string_const(query_instance_t  *output,
                                    const char        *input,
                                    query_type_list_t *query_type_list,
                                    GPtrArray         *aux) {
    char *buffer = strdup(input);
    if (!buffer)
        return 1;

    int retval = query_parser_parse_string(output, buffer, query_type_list, aux);

    free(buffer);
    return retval;
}
