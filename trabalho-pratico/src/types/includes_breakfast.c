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
 * @file  includes_breakfast.c
 * @brief Implementation of methods in include/types/includes_breakfast.h
 *
 * ### Examples
 * See [the header file's documentation](@ref includes_breakfast_example).
 */
#include <ctype.h>
#include <string.h>

#include "types/includes_breakfast.h"

int includes_breakfast_from_string(includes_breakfast_t *output, const char *input) {
    /* Inline hardcoded structure for better performance */
    const size_t len = strlen(input);
    switch (len) {
        case 0:
            *output = INCLUDES_BREAKFAST_FALSE;
            return 0;
        case 1: {
            const int l = tolower(input[0]);
            if (l == '0' || l == 'f') {
                *output = INCLUDES_BREAKFAST_FALSE;
            } else if (l == '1' || l == 't') {
                *output = INCLUDES_BREAKFAST_TRUE;
            } else {
                return 1;
            }
            return 0;
        }
        case 4: {
            /* Inline lowercase comparison with "true" */
            const int ret = tolower(input[0]) == 't' && tolower(input[1]) == 'r' &&
                            tolower(input[2]) == 'u' && tolower(input[3]) == 'e';
            if (ret)
                *output = INCLUDES_BREAKFAST_TRUE;
            return !ret;
        }
        case 5: {
            /* Inline lowercase comparison with "false" */
            const int ret = tolower(input[0]) == 'f' && tolower(input[1]) == 'a' &&
                            tolower(input[2]) == 'l' && tolower(input[3]) == 's' &&
                            tolower(input[4]) == 'e';
            if (ret)
                *output = INCLUDES_BREAKFAST_FALSE;
            return !ret;
        }
        default:
            return 1;
    }
}

void includes_breakfast_sprintf(char *output, includes_breakfast_t breakfast) {
    strcpy(output, breakfast == INCLUDES_BREAKFAST_TRUE ? "True" : "False");
}
