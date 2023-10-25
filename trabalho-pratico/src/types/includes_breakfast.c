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
 */
#include <string.h>

#include "types/includes_breakfast.h"

int includes_breakfast_from_string(includes_breakfast_t *output, const char *input) {
    if (*input == '\0') {
        *output = INCLUDES_BREAKFAST_NO_INPUT;
        return 0;
    } else if (strcmp(input, "f") == 0) {
        *output = INCLUDES_BREAKFAST_F;
        return 0;
    } else if (strcmp(input, "false") == 0) {
        *output = INCLUDES_BREAKFAST_FALSE;
        return 0;
    } else if (strcmp(input, "t") == 0) {
        *output = INCLUDES_BREAKFAST_T;
        return 0;
    } else if (strcmp(input, "true") == 0) {
        *output = INCLUDES_BREAKFAST_TRUE;
        return 0;
    } else if (strcmp(input, "1") == 0) {
        *output = INCLUDES_BREAKFAST_1;
        return 0;
    } else if (strcmp(input, "0") == 0) {
        *output = INCLUDES_BREAKFAST_0;
        return 0;
    }
    return 1;
}

void includes_breakfast_sprintf(char *output, includes_breakfast_t breakfast) {
    const char *source = "";

    switch (breakfast) {
        case INCLUDES_BREAKFAST_0:
            source = "0";
            break;
        case INCLUDES_BREAKFAST_1:
            source = "1";
            break;
        case INCLUDES_BREAKFAST_NO_INPUT:
            source = "";
            break;
        case INCLUDES_BREAKFAST_F:
            source = "f";
            break;
        case INCLUDES_BREAKFAST_FALSE:
            source = "false";
            break;
        case INCLUDES_BREAKFAST_T:
            source = "t";
            break;
        case INCLUDES_BREAKFAST_TRUE:
            source = "true";
            break;
    }

    strcpy(output, source);
}
