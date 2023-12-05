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
 * @file  dataset_parser.c
 * @brief Implementation of methods in include/dataset/dataset_parser.h
 *
 * ### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "dataset/dataset_parser.h"
#include "utils/stream_utils.h"

/**
 * @struct dataset_parser_grammar
 * @brief  The grammar definition for a dataset parser.
 *
 * @var dataset_parser_grammar::token_grammar
 *     @brief Grammar to use to parse single tokens with ::fixed_n_delimiter_parser_parse_string.
 * @var dataset_parser_grammar::before_parse_callback
 *     @brief Callback called before parsing each token.
 * @var dataset_parser_grammar::token_callback
 *     @brief Callback called after processing each token.
 * @var dataset_parser_grammar::delimiter
 *     @brief Separator between first-order tokens (e.g.: ``'\n'`` for CSV files).
 */
struct dataset_parser_grammar {
    fixed_n_delimiter_parser_grammar_t        *token_grammar;
    dataset_parser_token_before_parse_callback before_parse_callback;
    dataset_parser_token_callback              token_callback;
    char                                       delimiter;
};

/**
 * @struct dataset_parser_t
 * @brief  A parser for datasets.
 *
 * @var dataset_parser_t::grammar
 *     @brief Grammar that defines the parser.
 * @var dataset_parser_t::user_data
 *     @brief Data to be passed to callbacks in ::dataset_parser_t::grammar
 *     @details Not owned by this `struct`.
 * @var dataset_parser_t::first_token_passed
 *     @brief If the first token (to be ignored) has yet been processed.
 */
typedef struct {
    const dataset_parser_grammar_t *grammar;
    void                           *user_data;
    int                             first_token_passed;
} dataset_parser_t;

dataset_parser_grammar_t *
    dataset_parser_grammar_new(char                                       first_order_delimiter,
                               const fixed_n_delimiter_parser_grammar_t  *token_grammar,
                               dataset_parser_token_before_parse_callback before_parse_callback,
                               dataset_parser_token_callback              token_callback) {

    dataset_parser_grammar_t *grammar = malloc(sizeof(dataset_parser_grammar_t));
    if (!grammar) {
        return NULL;
    }

    grammar->delimiter     = first_order_delimiter;
    grammar->token_grammar = fixed_n_delimiter_parser_grammar_clone(token_grammar);
    if (!grammar->token_grammar) {
        free(grammar);
        return NULL;
    }
    grammar->before_parse_callback = before_parse_callback;
    grammar->token_callback        = token_callback;

    return grammar;
}

dataset_parser_grammar_t *dataset_parser_grammar_clone(const dataset_parser_grammar_t *grammar) {
    dataset_parser_grammar_t *new_grammar = malloc(sizeof(dataset_parser_grammar_t));
    if (!new_grammar)
        return NULL;

    memcpy(new_grammar, grammar, sizeof(dataset_parser_grammar_t));
    new_grammar->token_grammar = fixed_n_delimiter_parser_grammar_clone(grammar->token_grammar);
    if (!new_grammar->token_grammar) {
        free(new_grammar);
        return NULL;
    }

    return new_grammar;
}

void dataset_parser_grammar_free(dataset_parser_grammar_t *grammar) {
    fixed_n_delimiter_parser_grammar_free(grammar->token_grammar);
    free(grammar);
}

/**
 * @brief   Callback for every token parsed.
 * @details Auxiliary function for ::dataset_parser_parse, responsible for calling
 *          ::fixed_n_delimiter_parser_parse_string with the correct data.
 *
 * @param user_data A pointer to a ::dataset_parser_t.
 * @param token     The token to be parsed.
 */
int __parse_stream_iter(void *user_data, char *token) {
    dataset_parser_t *parser = (dataset_parser_t *) user_data;

    if (!parser->first_token_passed) {
        parser->first_token_passed = 1;
        return 0;
    }

    int before_parse_ret = parser->grammar->before_parse_callback(parser->user_data, token);
    if (before_parse_ret)
        return before_parse_ret;

    int parser_ret = fixed_n_delimiter_parser_parse_string(token,
                                                           parser->grammar->token_grammar,
                                                           parser->user_data);
    return parser->grammar->token_callback(parser->user_data, parser_ret);
}

int dataset_parser_parse(FILE *file, const dataset_parser_grammar_t *grammar, void *user_data) {
    dataset_parser_t parser = {.grammar = grammar, .user_data = user_data, .first_token_passed = 0};

    int retval = stream_tokenize(file, grammar->delimiter, __parse_stream_iter, &parser);
    if (retval == STREAM_TOKENIZE_RET_ALLOCATION_FAILURE)
        return DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE;

    return 0;
}
