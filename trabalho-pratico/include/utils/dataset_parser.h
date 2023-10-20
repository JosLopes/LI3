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
 * @file    dataset_parser.h
 * @brief   Contains utility methods for parsing datasets, seperated by a 
 *          single-character delimiter.
 * @details Useful for parsing CSV files.
 * 
 * @anchor dataset_parser_examples
 * ### Examples
 * 
 * Note that in this section we expand the example used in 
 * [the fixed_n_delimiter_parser examples](@ref fixed_n_delimiter_parser_examples).
 * 
 * Suppose we want to parse from an CSV file with the following contents:
 * ```
 * José Silva;60;176
 * José Matos;20;184
 * ```
 * 
 * ```c
 * typedef struct {
 *     char *name;
 *     int   age, height;
 * } person_t;
 * 
 * typedef struct {
 *     int        n;
 *     person_t   *person_array;
 * } person_parser_helper;
 * 
 * int parse_name(void *user_data, char *token, size_t ntoken) {
 *     (void) ntoken;
 *     person_parser_helper *helper = (person_parser_helper *)user_data;
 * 
 *     // Copy string to another buffer
 *     char *name_copy                = string_duplicate(token);
 *     helper->person_array[helper->n].name = name_copy;
 *     return 0;
 * }
 * 
 * int parse_int(void *user_data, char *token, size_t ntoken) {
 *     person_parser_helper *helper = (person_parser_helper *)user_data;
 * 
 *     int value = atoi(token);
 *     if (value <= 0) {
 *         fputs("Integer parsing failure!\n", stderr);
 *         return 1;
 *     }
 * 
 *     if (ntoken == 1) {
 *         helper->person_array[helper->n].age     = value;
 *     } else if (ntoken == 2) {
 *         helper->person_array[helper->n].height  = value;
 *         helper->n++;
 *     }
 *     return 0;
 * }
 * 
 * int main(void) {
 *     FILE *file = fopen("testfiles/testfile.txt", "r");
 * 
 *     fixed_n_delimiter_parser_iter_callback_t grammar_callbacks[3] = {parse_name,
 *                                                                      parse_int,
 *                                                                      parse_int};
 *     fixed_n_delimiter_parser_grammar_t      *grammar =
 *         fixed_n_delimiter_parser_grammar_new(';', 3, grammar_callbacks);
 * 
 *     person_parser_helper *helper = malloc(sizeof(person_parser_helper));
 *     helper->person_array = malloc(sizeof(person_t)*2);
 *     helper->n = 0;
 * 
 *     parse_dataset(file, '\n', grammar, helper);
 * 
 *     printf("PERSON1: %s, %d, %d\n", helper->person_array[0].name, helper->person_array[0].age, helper->person_array[0].height);
 *     printf("PERSON1: %s, %d, %d\n", helper->person_array[1].name, helper->person_array[1].age, helper->person_array[1].height);
 *  
 *     free(helper);
 *     fclose(file);
 *     fixed_n_delimiter_parser_grammar_free(grammar);
 *     return 0;
 * }
 * ```
 * 
 * We first define a grammar for a CSV (``delimiter = ';'``) with a string and two integers
 * (parsed by `parse_name` and `parse_int`, respectively), forming `grammar_callbacks`, of which
 * there are `3`.
 * 
 * First we parse the CSV file using the **new-line character**. Then this calls `fixed_n_delimiter_parser_parse_string`
 * to parse each line of the CSV file:
 *   - `fixed_n_delimiter_parser_parse_string("José Silva;60;176", grammar, helper)`;
 *   - `fixed_n_delimiter_parser_parse_string("José Matos;20;184", grammar, helper)`;
 * 
 * For errors and other results, refer to [the fixed_n_delimiter_parser examples](@ref fixed_n_delimiter_parser_examples).
 */

#ifndef DATASET_PARSER_H
#define DATASET_PARSER_H

#include "utils/fixed_n_delimiter_parser.h"

/**
 * @brief   The definition for data to be used while parsing the file.
 */
typedef struct stream_iter_data stream_iter_data;

/**
 * @brief Creates data to be used in ::__parse_stream_iter.
 *
 * @param grammar   Grammar that defines the parser to be used.
 * @param user_data Pointer passed to every callback in @p grammar, so that they can edit the
 *                  program's state.
 *
 * @return `malloc`-allocated ::stream_iter_data (or `NULL` on allocation failure).
 *         This value is owned by the function caller, so you must free it with
 *         ::stream_iter_data_free after you're done using it.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
stream_iter_data *stream_iter_data_new(fixed_n_delimiter_parser_grammar_t *grammar,
                                       void                               *user_data);

/**
 * @brief Frees memory allocated by ::stream_iter_data_new.
 * @param grammar Grammar allocated by ::stream_iter_data_new.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
void stream_iter_data_free(stream_iter_data *data);

/**
 * @brief Parses a file, using a `delimiter` and a parser defined by @p grammar.
 *
 * @param file      File to parse, that that will be modified for this function to work.
 * @param delimiter Character to separate the file. It won't be part of those tokens.
 * @param grammar   Grammar that defines the parser to be used.
 * @param user_data Pointer passed to every callback in @p grammar, so that they can edit the
 *                  program's state.
 *
 * @returns `0` on success. Other values are allowed, and happen when either there was an
 *          error on the grammar parser, or one of the callbacks interrupts parsing with 
 *          a non-`0` return value, then returned by this method.
 *
 * #### Examples
 * See [the header file's documentation](@ref dataset_parser_examples).
 */
int parse_dataset(FILE                               *file,
                  char                                delimiter,
                  fixed_n_delimiter_parser_grammar_t *grammar,
                  void                               *user_data);

#endif
