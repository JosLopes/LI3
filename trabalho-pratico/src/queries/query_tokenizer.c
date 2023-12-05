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
 * @file  query_tokenizer.c
 * @brief Implementation of methods in include/queries/query_tokenizer.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_tokenizer_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "queries/query_tokenizer.h"
#include "utils/string_utils.h"

/**
 * @struct query_tokenizer_data_t
 * @brief Contains the tokenizer's state.
 *
 * @var query_tokenizer_data_t::user_data
 *     @brief `user_data` parameter in ::query_tokenizer_tokenize.
 * @var query_tokenizer_data_t::callback
 *     @brief `callback` parameter in ::query_tokenizer_tokenize.
 * @var query_tokenizer_data_t::quote_token
 *     @brief The pointer to the beginning of the current token if is starts with ``'\"'``, `NULL`
 *            otherwise.
 */
typedef struct {
    void                    *user_data;
    tokenize_iter_callback_t callback;

    char *quote_token;
} query_tokenizer_data_t;

/**
 * @brief Handles space-separated tokens from a string.
 * @details Auxiliary method for ::query_tokenizer_tokenize.
 *
 * @param tokenizer_data A pointer to a ::query_tokenizer_data_t.
 * @param token          Space-separated token to process (take quotes into account).
 */
int __query_tokenizer_handle_space_split(void *tokenizer_data, char *token) {
    query_tokenizer_data_t *tokenizer = (query_tokenizer_data_t *) tokenizer_data;

    if (*token == '\0') /* Skip empty tokens */
        return 0;

    if (*token == '"')
        tokenizer->quote_token = token + 1;

    if (tokenizer->quote_token) {
        size_t token_length = strlen(token);
        if (token[token_length - 1] == '"') {

            token[token_length - 1] = '\0';
            int cb_result = tokenizer->callback(tokenizer->user_data, tokenizer->quote_token);

            token[token_length - 1] = '"'; /* Restore string */
            tokenizer->quote_token  = NULL;

            if (cb_result)
                return cb_result;
        }
    } else {
        int cb_result = tokenizer->callback(tokenizer->user_data, token);
        if (cb_result)
            return cb_result;
    }

    return 0;
}

int query_tokenizer_tokenize(char *input, tokenize_iter_callback_t callback, void *user_data) {
    query_tokenizer_data_t tokenizer_data = {.user_data   = user_data,
                                             .callback    = callback,
                                             .quote_token = NULL};

    int retval = string_tokenize(input, ' ', __query_tokenizer_handle_space_split, &tokenizer_data);
    if (retval)
        return retval;

    (void) callback;
    (void) user_data;
    return 0;
}

int query_tokenizer_tokenize_const(const char              *input,
                                   tokenize_iter_callback_t callback,
                                   void                    *user_data) {
    char *buffer = strdup(input);
    if (!buffer)
        return QUERY_TOKENIZER_TOKENIZE_CONST_RET_FAILED_MALLOC;

    int retval = query_tokenizer_tokenize(buffer, callback, user_data);

    free(buffer);
    return retval;
}
