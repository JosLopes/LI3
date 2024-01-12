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
 * @file  fixed_n_delimiter_parser.c
 * @brief Implementation of methods in include/utils/fixed_n_delimiter_parser.h
 *
 * ### Examples
 * See [the header file's documentation](@ref fixed_n_delimiter_parser_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "utils/fixed_n_delimiter_parser.h"
#include "utils/string_utils.h"

/**
 * @struct fixed_n_delimiter_parser_grammar
 * @brief  The grammar definition for a parser of strings with a known number of data points,
 *         separated by a single-character delimiter.
 *
 * @var fixed_n_delimiter_parser_grammar::delimiter
 *     @brief Separator between data points (e.g.: ``'/'`` for dates, ``','`` for CSV lines, ...).
 * @var fixed_n_delimiter_parser_grammar::n
 *     @brief Number of expected data points (e.g.: `3` for dates, year, month and day).
 * @var fixed_n_delimiter_parser_grammar::callbacks
 *     @brief A callback for each token to be parsed. There have a length of
 *            ::fixed_n_delimiter_parser_grammar::n.
 */
struct fixed_n_delimiter_parser_grammar {
    char                                      delimiter;
    size_t                                    n;
    fixed_n_delimiter_parser_iter_callback_t *callbacks;
};

/**
 * @struct fixed_n_delimiter_parser_t
 * @brief The state of a parser of strings with a known number of data points, separated by a
 *        single-character delimiter.
 *
 * @var fixed_n_delimiter_parser_t::grammar
 *     @brief Grammar that defines the parser.
 * @var fixed_n_delimiter_parser_t::token_count
 *     @brief Current state of the parser (number of tokens already read).
 * @var fixed_n_delimiter_parser_t::user_data
 *     @brief Data to be passed to callbacks in ::fixed_n_delimiter_parser_t::grammar
 *     @details Not owned by this `struct`.
 */
typedef struct {
    const fixed_n_delimiter_parser_grammar_t *grammar;
    size_t                                    token_count;
    void                                     *user_data;
} fixed_n_delimiter_parser_t;

fixed_n_delimiter_parser_grammar_t *fixed_n_delimiter_parser_grammar_new(
    char                                           delimiter,
    size_t                                         n,
    const fixed_n_delimiter_parser_iter_callback_t callbacks[n]) {

    fixed_n_delimiter_parser_grammar_t *grammar =
        malloc(sizeof(fixed_n_delimiter_parser_grammar_t));
    if (!grammar)
        return NULL;

    grammar->delimiter = delimiter;
    grammar->n         = n;
    grammar->callbacks = malloc(sizeof(fixed_n_delimiter_parser_iter_callback_t) * n);
    if (!grammar->callbacks) {
        free(grammar);
        return NULL;
    }
    memcpy(grammar->callbacks, callbacks, n * sizeof(fixed_n_delimiter_parser_iter_callback_t));

    return grammar;
}

fixed_n_delimiter_parser_grammar_t *
    fixed_n_delimiter_parser_grammar_clone(const fixed_n_delimiter_parser_grammar_t *grammar) {

    fixed_n_delimiter_parser_grammar_t *new_grammar =
        malloc(sizeof(fixed_n_delimiter_parser_grammar_t));
    if (!new_grammar)
        return NULL;

    memcpy(new_grammar, grammar, sizeof(fixed_n_delimiter_parser_grammar_t));

    new_grammar->callbacks = malloc(sizeof(fixed_n_delimiter_parser_iter_callback_t) * grammar->n);
    if (!new_grammar->callbacks) {
        free(new_grammar);
        return NULL;
    }
    memcpy(new_grammar->callbacks,
           grammar->callbacks,
           sizeof(fixed_n_delimiter_parser_iter_callback_t) * grammar->n);

    return new_grammar;
}

void fixed_n_delimiter_parser_grammar_free(fixed_n_delimiter_parser_grammar_t *grammar) {
    free(grammar->callbacks);
    free(grammar);
}

/**
 * @brief   Callback for every token parsed.
 * @details Auxiliary function for ::fixed_n_delimiter_parser_parse_string, responsible for
 *          choosing the right parser grammar callback for the token and checking that the number
 *          of data points didn't exceed the upper limit.
 *
 * @param parser_data A pointer to a ::fixed_n_delimiter_parser_t.
 * @param token       The token to be parsed.
 *
 * @return `0` when successful, to continue with iteration, or another number to stop it.
 */
int __parse_string_iter(void *parser_data, char *token) {
    fixed_n_delimiter_parser_t *parser = (fixed_n_delimiter_parser_t *) parser_data;

    if (parser->token_count >= parser->grammar->n)
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS;

    const fixed_n_delimiter_parser_iter_callback_t callback =
        parser->grammar->callbacks[parser->token_count];
    const int retval = callback(parser->user_data, token, parser->token_count);

    parser->token_count++;
    return retval;
}

int fixed_n_delimiter_parser_parse_string(char                                     *input,
                                          const fixed_n_delimiter_parser_grammar_t *grammar,
                                          void                                     *user_data) {

    fixed_n_delimiter_parser_t parser = {.grammar     = grammar,
                                         .token_count = 0,
                                         .user_data   = user_data};

    const int tok_ret = string_tokenize(input, grammar->delimiter, __parse_string_iter, &parser);
    if (tok_ret)
        return tok_ret;

    if (parser.token_count < grammar->n) {
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS;
    } else {
        return 0;
    }
}

int fixed_n_delimiter_parser_parse_string_const(const char                               *input,
                                                const fixed_n_delimiter_parser_grammar_t *grammar,
                                                void *user_data) {
    char *buffer = strdup(input);
    if (!buffer)
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_CONST_RET_MALLOC_FAILURE;

    const int retval = fixed_n_delimiter_parser_parse_string(buffer, grammar, user_data);

    free(buffer);
    return retval;
}
