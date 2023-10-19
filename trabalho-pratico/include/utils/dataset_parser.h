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
 * @file dataset_parser.h
 * @brief Contains utility methods for dealing with file streams.
 * 
 */

#ifndef DATASET_PARSER_H
#define DATASET_PARSER_H

#include "utils/fixed_n_delimiter_parser.h"

typedef struct stream_iter_data stream_iter_data;

stream_iter_data *
    stream_iter_data_new(fixed_n_delimiter_parser_grammar_t *grammar,
                         void                               *user_data);

void stream_iter_data_free(stream_iter_data *data);

int __parse_stream_iter(void *user_data, char* token);

int parse_dataset(FILE                               *file,
                  char                                delimiter,
                  fixed_n_delimiter_parser_grammar_t *grammar,
                  void                               *user_data);


#endif

// TODO: Document