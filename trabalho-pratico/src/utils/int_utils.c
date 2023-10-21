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
 * @file  int_utils.c
 * @brief Implementation of methods in include/utils/int_utils.h
 *
 * ### Examples
 * See [the header file's documentation](@ref int_utils_examples).
 */

#include "utils/int_utils.h"

int int_utils_parse_positive(uint64_t *output, const char *input) {
    uint64_t acc       = 0;
    int      not_empty = 0;

    while (*input) {
        not_empty = 1;
        acc *= 10;

        if ('0' <= *input && *input <= '9')
            acc += (uint64_t) (*input - '0');
        else
            return 1;

        input++;
    }

    if (!not_empty)
        return 1;

    *output = acc;
    return 0;
}
