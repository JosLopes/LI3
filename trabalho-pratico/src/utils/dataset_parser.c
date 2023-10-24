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
 * @brief Implementation of methods in include/utils/dataset_parser.h
 *
 * ### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils/dataset_parser.h"
#include "utils/stream_utils.h"

/**
 * @struct dataset_parser_grammar_t
 * @brief  The grammar definition for a dataset parser.
 *
 * @var dataset_parser_grammar_t::token_grammar
 *     @brief Grammar to use to parse single tokens with ::fixed_n_delimiter_parser_parse_string.
 * @var dataset_parser_grammar_t::token_callback
 *     @brief Callback called after processing each token.
 * @var dataset_parser_grammar_t::delimiter
 *     @brief Separator between first-order tokens (e.g.: ``'\n'`` for CSV files).
 */
struct dataset_parser_grammar_t {
    fixed_n_delimiter_parser_grammar_t *token_grammar;
    dataset_parser_token_callback       token_callback;
    char                                delimiter;
};

/**
 * @struct dataset_parser
 * @brief A parser for datasets.
 *
 * @var dataset_parser_t::grammar
 *     @brief Grammar that defines the parser.
 *     @details Not owned by this `struct`.
 * @var dataset_parser_t::user_data
 *     @brief Data to be passed to callbacks in ::dataset_parser_t::grammar
 *     @details Not owned by this `struct`.
 * @var dataset_parser_t::first_token_passed
 *     @brief If the first token (to be ignored) has yet been processed.
 */
typedef struct {
    dataset_parser_grammar_t *grammar;
    void                     *user_data;
    int                       first_token_passed;
} dataset_parser_t;

dataset_parser_grammar_t *
    dataset_parser_grammar_new(char                                first_order_delimiter,
                               fixed_n_delimiter_parser_grammar_t *token_grammar,
                               dataset_parser_token_callback       token_callback) {

    dataset_parser_grammar_t *grammar = malloc(sizeof(struct dataset_parser_grammar_t));
    if (!grammar) {
        return NULL;
    }

    grammar->delimiter      = first_order_delimiter;
    grammar->token_grammar  = token_grammar;
    grammar->token_callback = token_callback;

    return grammar;
}

void dataset_parser_grammar_free(dataset_parser_grammar_t *grammar) {
    free(grammar->token_grammar);
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

    int parser_ret = fixed_n_delimiter_parser_parse_string(token,
                                                           parser->grammar->token_grammar,
                                                           parser->user_data);
    return parser->grammar->token_callback(parser->user_data, parser_ret);
}

int dataset_parser_parse(FILE *file, dataset_parser_grammar_t *grammar, void *user_data) {
    dataset_parser_t parser = {.grammar = grammar, .user_data = user_data, .first_token_passed = 0};

    int retval = stream_tokenize(file, grammar->delimiter, __parse_stream_iter, &parser);
    if (retval == STREAM_TOKENIZE_RET_ALLOCATION_FAILURE)
        return DATASET_PARSER_PARSE_RET_ALLOCATION_FAILURE;

    return 0;
}
