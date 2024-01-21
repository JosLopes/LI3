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

#include <glib.h>

#include "queries/query_parser.h"
#include "queries/query_tokenizer.h"
#include "queries/query_type_list.h"
#include "utils/int_utils.h"

/**
 * @struct query_parser_data_t
 * @brief  State of a query parser.
 *
 * @var query_parser_data_t::output
 *     @brief Query being currently parsed.
 * @var query_parser_data_t::args
 *     @brief Array to store pointers to query arguments (`char *`) to.
 * @var query_parser_data_t::first_token_parsed
 *     @brief Whether the first token (containg the query type) has already been parsed.
 * @var query_parser_data_t::last_terminator
 *     @brief Where a null terminator (``'\0'``) should be placed, so that the previous token ends;
 *            `NULL` if no token has yet been parsed.
 */
typedef struct {
    query_instance_t *const output;
    GPtrArray              *args;
    int                     first_token_parsed;
    char                   *last_terminator;
} query_parser_data_t;

/**
 * @brief   Callback for every token in the query.
 * @details Parses the first token and adds the remaining ones to an array.
 *
 * @param user_data A pointer to a ::query_parser_data_t.
 * @param token     Current query token being parsed.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 */
int __query_parser_tokenize_callback(void *user_data, char *token) {
    query_parser_data_t *const parser = user_data;

    if (!parser->first_token_parsed) { /* First argument: query number */

        /* Check if query is formatted */
        const size_t token_len = strlen(token);
        if (token_len == 0)
            return 1;

        if (token[token_len - 1] == 'F') {
            query_instance_set_formatted(parser->output, 1);
            token[token_len - 1] = '\0';
        } else {
            query_instance_set_formatted(parser->output, 0);
        }

        /* Parse number of query */
        uint64_t                  query_type;
        const int                 retval = int_utils_parse_positive(&query_type, token);
        const query_type_t *const type =
            retval ? NULL : query_type_list_get_by_index((size_t) query_type);
        if (retval || !type) {
            if (token[token_len - 1] == '\0') /* Restore previous string form */
                token[token_len - 1] = 'F';
            return 1;
        }
        query_instance_set_type(parser->output, type);

        if (token[token_len - 1] == '\0') /* Restore previous string form */
            token[token_len - 1] = 'F';
        parser->first_token_parsed = 1;
    } else {
        if (parser->last_terminator)
            *parser->last_terminator = '\0';
        parser->last_terminator = token + strlen(token);
        g_ptr_array_add(parser->args, token);
    }
    return 0;
}

int query_parser_parse_string(query_instance_t *output, char *input, GPtrArray *aux) {
    query_parser_data_t parser_data = {.output             = output,
                                       .first_token_parsed = 0,
                                       .last_terminator    = NULL};
    if (aux) {
        g_ptr_array_set_size(aux, 0);
        parser_data.args = aux;
    } else {
        parser_data.args = g_ptr_array_new();
    }

    /* Query type parsing */
    const int retval =
        query_tokenizer_tokenize(input, __query_parser_tokenize_callback, &parser_data);
    if (retval || !parser_data.first_token_parsed) {
        if (!aux)
            g_ptr_array_free(parser_data.args, TRUE);
        return 1;
    }

    if (parser_data.last_terminator)
        *parser_data.last_terminator = '\0';

    /* Argument parsing */
    const query_type_t *const                   query_type = query_instance_get_type(output);
    const query_type_parse_arguments_callback_t parse_cb =
        query_type_get_parse_arguments_callback(query_type);
    void *const argument_data =
        parse_cb(parser_data.args->len, (char *const *) parser_data.args->pdata);

    if (argument_data) {
        query_instance_set_argument_data(output, argument_data);

        const query_type_free_arguments_callback_t free_cb =
            query_type_get_free_arguments_callback(query_type);
        free_cb(argument_data);
    }

    /* Restore string */
    for (ssize_t i = 0; i < parser_data.args->len; ++i) {
        char *const str = g_ptr_array_index(parser_data.args, i);
        char *const end = str + strlen(str);

        if (*(str - 1) == '"')
            *end = '"';
        else if (i != parser_data.args->len - 1)
            *end = ' ';
    }

    if (!aux)
        g_ptr_array_unref(parser_data.args);
    return (argument_data == NULL);
}

int query_parser_parse_string_const(query_instance_t *output, const char *input, GPtrArray *aux) {
    char *const buffer = strdup(input);
    if (!buffer)
        return QUERY_PARSER_PARSE_CONST_RET_FAILED_MALLOC;

    const int retval = query_parser_parse_string(output, buffer, aux);
    free(buffer);
    return retval != 0;
}
