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
 * @file    fixed_n_delimiter_parser.h
 * @brief   A parser for strings with a known number of data points, separated by a
 *          single-character delimiter.
 * @details Useful for structures such as lines of CSV tables. In practice, this is used for
 *          [dates](@ref date.h), [times](@ref daytime.h), and
 *          [input datasets](@ref dataset_parser.h).
 *
 * @anchor fixed_n_delimiter_parser_examples
 * ### Examples
 *
 * In this section of examples, ::fixed_n_delimiter_parser_parse_string_const was used, but
 * ::fixed_n_delimiter_parser_parse_string presents the same behavior, with the exception that the
 * @p input must be modifiable.
 *
 * A complete example of ::fixed_n_delimiter_parser_parse_string_const follows. Suppose we want to
 * parse information about a person in a CSV string.
 *
 * ```
 * #define PERSON_DATA "José Silva,60,176" // Name, age and height
 *
 * typedef struct {
 *     char *name;
 *     int   age, height;
 * } person_t;
 *
 * int parse_name(void *user_data, char *token, size_t ntoken) {
 *     (void) ntoken;
 *
 *     // Copy string to another buffer
 *     char *name_copy = strdup(token);
 *     if (!name_copy)
 *         return 1;
 *     ((person_t *) user_data)->name = name_copy;
 *     return 0;
 * }
 *
 * int parse_int(void *user_data, char *token, size_t ntoken) {
 *     int value = atoi(token);
 *
 *     if (value < 0) {
 *         fputs("Integer parsing failure!\n", stderr);
 *         return 1;
 *     }
 *
 *     if (ntoken == 1) {
 *         ((person_t *) user_data)->age = value;
 *     } else if (ntoken == 2) {
 *         ((person_t *) user_data)->height = value;
 *     }
 *     return 0;
 * }
 *
 * int main(void) {
 *     const fixed_n_delimiter_parser_iter_callback_t grammar_callbacks[3] = {parse_name,
 *                                                                            parse_int,
 *                                                                            parse_int};
 *
 *     fixed_n_delimiter_parser_grammar_t *grammar =
 *         fixed_n_delimiter_parser_grammar_new(',', 3, grammar_callbacks);
 *
 *     person_t person = {0};
 *     int status = fixed_n_delimiter_parser_parse_string_const(PERSON_DATA, grammar, &person);
 *     if (status) {
 *         fprintf(stderr, "Parsing failure! (%d)\n", status);
 *     } else {
 *         printf("%s is %d and %dcm tall\n", person.name, person.age, person.height);
 *     }
 *
 *     if (person.name)
 *         free(person.name);
 *
 *     fixed_n_delimiter_parser_grammar_free(grammar);
 *     return status;
 * }
 * ```
 *
 * We first define a grammar for a CSV (``delimiter = ','``) with a string and two integers
 * (parsed by `parse_name` and `parse_int`, respectively), forming `grammar_callbacks`, of which
 * there are `3`.
 *
 * Parsing `PERSON_DATA` will result in the following 3 callbacks:
 *   - `parse_name(&person, "José Silva", 0)`;
 *   - `parse_int(&person, "60", 1)`;
 *   - `parse_int(&person, "176", 2)`;
 *
 * Note that not only does `parse_int` use `ntoken` to know where to write the parsed value to, but
 * it may also return `1`, stopping the parsing early due to invalid data. In this case,
 * ::fixed_n_delimiter_parser_parse_string_const will return `1`.
 *
 * Other errors are possible:
 *
 *  - ``#define PERSON_DATA "José Silva,60,176,12"`` -
 *    ::fixed_n_delimiter_parser_parse_string_const returns
 *    ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS;
 *
 *  - ``#define PERSON_DATA "José Silva,60"`` - ::fixed_n_delimiter_parser_parse_string_const
 *     returns ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS;
 *
 *  - ``#define PERSON_DATA "José Silva,-60"`` - ::fixed_n_delimiter_parser_parse_string_const
 *    returns `1`, because `parse_int` failed before trying to read the third token;
 *
 * When freeing the data in the end, keep in mind that, due to the possibility of parser failure,
 * not all data may have been initialized, hence the if statement relating to `person.name`.
 */

#ifndef FIXED_N_DELIMITER_PARSER_H
#define FIXED_N_DELIMITER_PARSER_H

#include <stddef.h>

/**
 * @brief Method that, in a ::fixed_n_delimiter_parser_grammar_t, is associated with an `n`-th
 *        token in a string, and is called when that token needs to be parsed.
 *
 * @param user_data Pointer provided to ::fixed_n_delimiter_parser_parse_string (or
 *                  ::fixed_n_delimiter_parser_parse_string_const), so that this callback can
 *                  modify the program's state.
 * @param token     The token that was read. In case you want to store @p token (or part of it) in
 *                  @p user_data, copy it first, as its lifetime is limited to this function.
 * @param ntoken    The number of the current token. May be useful when the same parser function is
 *                  associated with multiple data points.
 *
 * @return `0` on success, other value for immediate termination of parsing. It's recommeneded that
 *         these values are positive, as negative values have special meanings (see
 *         ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS,
 *         ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS and
 *         ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_CONST_RET_MALLOC_FAILURE).
 */
typedef int (*fixed_n_delimiter_parser_iter_callback_t)(void  *user_data,
                                                        char  *token,
                                                        size_t ntoken);

/**
 * @brief The grammar definition for a parser of strings with a known number of data points,
 *        separated by a single-character delimiter.
 */
typedef struct fixed_n_delimiter_parser_grammar fixed_n_delimiter_parser_grammar_t;

/**
 * @brief Creates a parser's grammar definition.
 *
 * @param delimiter Separator between data points (e.g.: ``'/'`` for dates, ``','`` for CSV lines,
 *                  ...).
 * @param n         Number of expected data points (e.g.: `3` for dates: a year, a month and a day).
 * @param callbacks A callback for each token to be parsed.
 *
 * @return A pointer to a ::fixed_n_delimiter_parser_grammar_t (or `NULL` on allocation
 *         failure). This value is owned by the function caller, so you must free it with
 *         ::fixed_n_delimiter_parser_grammar_free.
 *
 * #### Examples
 * See [the header file's documentation](@ref fixed_n_delimiter_parser_examples).
 */
fixed_n_delimiter_parser_grammar_t *fixed_n_delimiter_parser_grammar_new(
    char                                           delimiter,
    size_t                                         n,
    const fixed_n_delimiter_parser_iter_callback_t callbacks[n]);

/**
 * @brief Creates a deep clone of a grammar for this type of parser.
 * @param grammar Grammar to be cloned.
 *
* @return A pointer to a ::fixed_n_delimiter_parser_grammar_t (or `NULL` on allocation
 *         failure). This value is owned by the function caller, so you must free it with
 *         ::fixed_n_delimiter_parser_grammar_free.
 */
fixed_n_delimiter_parser_grammar_t *
    fixed_n_delimiter_parser_grammar_clone(const fixed_n_delimiter_parser_grammar_t *grammar);

/**
 * @brief Frees memory allocated by ::fixed_n_delimiter_parser_grammar_new.
 * @param grammar Grammar allocated by ::fixed_n_delimiter_parser_grammar_new.
 *
 * #### Examples
 * See [the header file's documentation](@ref fixed_n_delimiter_parser_examples).
 */
void fixed_n_delimiter_parser_grammar_free(fixed_n_delimiter_parser_grammar_t *grammar);

/**
 * @brief Value returned by ::fixed_n_delimiter_parser_parse_string (or
 *        ::fixed_n_delimiter_parser_parse_string_const) when there are more data points than
 *        expected.
 */
#define FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS -1

/**
 * @brief Value returned by ::fixed_n_delimiter_parser_parse_string (or
 *        ::fixed_n_delimiter_parser_parse_string_const) when there are less data points than
 *        expected.
 */
#define FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS -2

/** @brief Value returned by ::fixed_n_delimiter_parser_parse_string_const when `malloc` fails. */
#define FIXED_N_DELIMITER_PARSER_PARSE_STRING_CONST_RET_MALLOC_FAILURE -3

/**
 * @brief Parses a **MODIFIABLE** string using a parser defined by @p grammar.
 *
 * @param input     String to parse, that will be modified during parsing, but then restored to its
 *                  original form, assuming callbacks in @p grammar don't modify it.
 * @param grammar   Grammar that defines the parser to be used.
 * @param user_data Pointer passed to every callback in @p grammar, so that they can edit the
 *                  program's state.
 *
 * @returns
 *     - `0` on success;
 *     - ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS when there are too many
 *       data points;
 *     - ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS when there are two few
 *       data points;
 *     - Other values, when one of the callbacks interrupts parsing with a non-`0` return
 *       value, that is then returned by this method.

 * #### Examples
 * See [the header file's documentation](@ref fixed_n_delimiter_parser_examples).
 */
int fixed_n_delimiter_parser_parse_string(char                                     *input,
                                          const fixed_n_delimiter_parser_grammar_t *grammar,
                                          void                                     *user_data);

/**
 * @brief   Parses a string using a parser defined by @p grammar.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param input     String to parse.
 * @param grammar   Grammar that defines the parser to be used.
 * @param user_data Pointer passed to every callback in @p grammar, so that they can edit the
 *                  program's state.
 *
 * @returns
 *     - `0` on success;
 *     - ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_TOO_MANY_ITEMS when there are too many
 *       data points;
 *     - ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_RET_NOT_ENOUGH_ITEMS when there are two few
 *       data points;
 *     - ::FIXED_N_DELIMITER_PARSER_PARSE_STRING_CONST_RET_MALLOC_FAILURE when an allocation
 *       failure occurs;
 *     - Other values, when one of the callbacks interrupts parsing with a non-`0` return
 *       value, that is then returned by this method.
 *
 * #### Examples
 * See [the header file's documentation](@ref fixed_n_delimiter_parser_examples).
 */
int fixed_n_delimiter_parser_parse_string_const(const char                               *input,
                                                const fixed_n_delimiter_parser_grammar_t *grammar,
                                                void *user_data);

#endif
