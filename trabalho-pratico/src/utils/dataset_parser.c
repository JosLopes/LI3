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
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/stream_utils.h"

struct stream_iter_data {
    fixed_n_delimiter_parser_grammar_t *grammar;
    void *user_data;
};

stream_iter_data *
    stream_iter_data_new(fixed_n_delimiter_parser_grammar_t *grammar,
                         void                               *user_data) {
    
    stream_iter_data *data = malloc(sizeof(stream_iter_data));
    data->grammar = grammar;
    data->user_data = user_data;

    return data;
}

void stream_iter_data_free(stream_iter_data *data) {
    free(data);
}

int __parse_stream_iter(void *user_data, char* token) {
    stream_iter_data *data = (stream_iter_data *)user_data;
    int retval = fixed_n_delimiter_parser_parse_string(token, data->grammar, data->user_data);
    return retval;
}

int parse_dataset(FILE                               *file,
                  char                                delimiter,
                  fixed_n_delimiter_parser_grammar_t *grammar,
                  void                               *user_data) {

    stream_iter_data *data = stream_iter_data_new(grammar, user_data);

    stream_tokenize(file, delimiter, __parse_stream_iter, data);

    stream_iter_data_free(data);
    return 0;
}
