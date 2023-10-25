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
 * @file  country_code.c
 * @brief Implementation of methods in include/types/country_code.h
 *
 * ### Examples
 * See [the header file's documentation](@ref country_code_examples).
 */

#include <ctype.h>

#include "types/country_code.h"

int country_code_from_string(country_code_t *output, const char *input) {
    if (*input && *(input + 1) && !*(input + 2) && isalpha(*input) && isalpha(*(input + 1))) {
        *output = *(country_code_t *) input;
        return 0;
    }
    return 1;
}

void country_code_sprintf(char *output, country_code_t country) {
    *((uint16_t *) output) = country;
    output[2]              = '\0';
}
