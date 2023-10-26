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
 * @file  airport_code.c
 * @brief Implementation of methods in include/types/airport_code.h
 *
 * ### Examples
 * See [the header file's documentation](@ref airport_code_examples).
 */

#include <ctype.h>

#include "types/airport_code.h"

int airport_code_from_string(airport_code_t *output, const char *input) {
    if (*input && *(input + 1) && *(input + 2) && !*(input + 3) && isalpha(*input) &&
        isalpha(*(input + 1)) && isalpha(*(input + 2))) {

        *((char *) output)       = toupper(input[0]);
        *(((char *) output) + 1) = toupper(input[1]);
        *(((char *) output) + 2) = toupper(input[2]);
        *(((char *) output) + 3) = '\0';
        return 0;
    }
    return 1;
}

void airport_code_sprintf(char *output, airport_code_t airport) {
    *((airport_code_t *) output) = airport;
}
