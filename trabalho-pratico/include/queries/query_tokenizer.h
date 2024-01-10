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
 * @file    query_tokenizer.h
 * @brief   Tokenizer for queries that may contain quotes.
 * @details This parser is very limited, as it supports only the bare minimum to get queries
 *          working. The following items aren't supported:
 *
 *          - There is no quote escaping;
 *          - Quotes must be preceded and proceded by spaces (e.g.: ``Hello "a b c" Goodbye``).
 *
 *          Under normal circumstances, you should use a [query parser](@ref query_parser.h)
 *          instead.
 *
 * @anchor query_tokenizer_examples
 * ### Examples
 *
 * In the following example, queries are split by tokens, and a token is printed on each line.
 *
 * ```c
 * #include <stdio.h>
 * #include <stdlib.h>
 *
 * #include "queries/query_tokenizer.h"
 *
 * int query_token_callback(void *user_data, char *token) {
 *     (void) user_data;
 *     printf("%s\n", token);
 *     return 0;
 * }
 *
 * int main(void) {
 *     const char *queries[4] = {"88F Hello world",
 *                               "1 \"Hello, world\"",
 *                               "2 \"multiple   spaces\"",
 *                               "3 \"multiple quotes\" \"in this query\""};
 *
 *     for (size_t i = 0; i < 4; ++i) {
 *         int result = query_tokenizer_tokenize_const(queries[i], query_token_callback, NULL);
 *         if (result)
 *             fprintf(stderr, "Failed to parse query: %s\n", queries[i]);
 *         else
 *             putchar('\n');
 *     }
 *
 *     return 0;
 * }
 * ```
 */

#ifndef QUERY_TOKENIZER_H
#define QUERY_TOKENIZER_H

#include "utils/tokenize_iter_callback.h"

/**
 * @brief Splits a **MODIFIABLE** string into query tokens.
 *
 * @param input     String to tokenize, that that will be modified for this function to work, but
 *                  later restored to its original form, assuming @p callback doesn't modify it.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of @p callback, so that it can edit program state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_tokenizer_examples).
 */
int query_tokenizer_tokenize(char *input, tokenize_iter_callback_t callback, void *user_data);

/**
 * @brief Value returned by ::query_tokenizer_tokenize_const when `malloc` fails.
 */
#define QUERY_TOKENIZER_TOKENIZE_CONST_RET_FAILED_MALLOC -1

/**
 * @brief See ::query_tokenizer_tokenize_const, but this method applies to `const` strings.
 *
 * @details The current implementation allocates a writeable buffer and copies over the string
 *          before calling ::query_tokenizer_tokenize, so **it's very inefficient** and should not
 *          be used for large strings.
 *
 * @param input     String to tokenize.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of @p callback, so that it can edit program state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop. ::QUERY_TOKENIZER_TOKENIZE_CONST_RET_FAILED_MALLOC is returned on
 *         failure to allocate a writeable buffer the size of @p input.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_tokenizer_examples).
 */
int query_tokenizer_tokenize_const(const char              *input,
                                   tokenize_iter_callback_t callback,
                                   void                    *user_data);

#endif
