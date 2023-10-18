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
 * @brief The state of the parser is the number of tokens already parsed.
 */
typedef size_t fixed_n_delimiter_parser_state_t;

/**
 * @struct fixed_n_delimiter_parser_grammar_t
 * @brief  The grammar definition for a parser for strings with a known number of data points,
 *         separated by a single-character delimiter.
 *
 * @var fixed_n_delimiter_parser_grammar_t::delimiter
 *     @brief Separator between data points (e.g.: ``'/'`` for dates, ``','`` for CSV lines, ...).
 * @var fixed_n_delimiter_parser_grammar_t::n
 *     @brief Number of expected data points (e.g.: `3` for dates, year, month and day).
 * @var fixed_n_delimiter_parser_grammar_t::callbacks
 *     @brief A callback for each token to be parsed. There must be
 *     @ref fixed_n_delimiter_parser_grammar_t::n callbacks.
 *     @details Unconventially owned by this `struct` (when allocated, the buffer will contain
 *     space for storing the callbacks, so they'll get `free`d when the `struct` is `freed`).
 */
struct fixed_n_delimiter_parser_grammar_t {
    char                                      delimiter;
    size_t                                    n;
    fixed_n_delimiter_parser_iter_callback_t *callbacks;
};

/**
 * @struct fixed_n_delimiter_parser_t
 * @brief A parser for strings with a known number of data points, separated by a single-character
 *        delimiter.
 *
 * @var fixed_n_delimiter_parser_t::grammar
 *     @brief Grammar that defines the parser.
 *     @details Not owned by this `struct`.
 * @var fixed_n_delimiter_parser_t::state
 *     @brief Current state of the parser (number of tokens already read).
 * @var fixed_n_delimiter_parser_t::user_data
 *     @brief Data to be passed to callbacks in ::fixed_n_delimiter_parser_t::grammar
 *     @details Not owned by this `struct`.
 */
typedef struct {
    fixed_n_delimiter_parser_grammar_t *grammar;
    fixed_n_delimiter_parser_state_t    state;
    void                               *user_data;
} fixed_n_delimiter_parser_t;

fixed_n_delimiter_parser_grammar_t *
    fixed_n_delimiter_parser_grammar_new(char                                     delimiter,
                                         size_t                                   n,
                                         fixed_n_delimiter_parser_iter_callback_t callbacks[n]) {
    /*
     * Create a buffer that contains both grammar structure itself, and the array of callbacks
     * after that.
     *
     * +---------------------------------+
     * | GRAMMAR | CB1 | CB2 | ... | CBn |
     * +---------------------------------+
     */
    fixed_n_delimiter_parser_grammar_t *grammar =
        malloc(sizeof(fixed_n_delimiter_parser_grammar_t) +
               sizeof(fixed_n_delimiter_parser_iter_callback_t) * n);

    if (!grammar)
        return NULL;

    fixed_n_delimiter_parser_iter_callback_t *output_callbacks =
        (fixed_n_delimiter_parser_iter_callback_t *) (grammar + 1);

    grammar->delimiter = delimiter;
    grammar->n         = n;
    grammar->callbacks = output_callbacks;
    (void) memcpy(output_callbacks,
                  callbacks,
                  n * sizeof(fixed_n_delimiter_parser_iter_callback_t *));

    return grammar;
}

void fixed_n_delimiter_parser_grammar_free(fixed_n_delimiter_parser_grammar_t *grammar) {
    free(grammar);
}

/**
 * @brief Callback for every token parsed.
 * @details Auxiliary function for ::fixed_n_delimiter_parser_parse_string, responsible for
 *          choosing the right parser grammar callback for the token and checking that the number
 *          of data points didn't exceed the upper limit.
 *
 * @param parser_data A pointer to a ::fixed_n_delimiter_parser_t.
 * @param token       The token to be parsed.
 */
int __parse_string_iter(void *parser_data, char *token) {
    fixed_n_delimiter_parser_t *parser = (fixed_n_delimiter_parser_t *) parser_data;

    if (parser->state >= parser->grammar->n)
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS;

    int retval = parser->grammar->callbacks[parser->state](parser->user_data, token, parser->state);

    parser->state++;
    return retval;
}

int fixed_n_delimiter_parser_parse_string(char                               *input,
                                          fixed_n_delimiter_parser_grammar_t *grammar,
                                          void                               *user_data) {

    fixed_n_delimiter_parser_t parser = {.grammar = grammar, .state = 0, .user_data = user_data};

    int tok_ret = string_tokenize(input, grammar->delimiter, __parse_string_iter, &parser);
    if (tok_ret)
        return tok_ret;

    if (parser.state < grammar->n) {
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS;
    } else {
        return 0;
    }
}

int fixed_n_delimiter_parser_parse_string_const(const char                         *input,
                                                fixed_n_delimiter_parser_grammar_t *grammar,
                                                void                               *user_data) {
    char *buffer = string_duplicate(input);
    if (!buffer)
        return FIXED_N_DELIMITER_PARSER_PARSE_STRING_CONST_RET_MALLOC_FAILURE;

    int retval = fixed_n_delimiter_parser_parse_string(buffer, grammar, user_data);

    free(buffer);
    return retval;
}
