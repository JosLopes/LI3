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
#include <ctype.h>
#include <string.h>

#include "types/includes_breakfast.h"

/** @brief Value of `strlen("false")`. */
#define INCLUDES_BREAKFAST_STRLEN_FALSE 5

int includes_breakfast_from_string(includes_breakfast_t *output, const char *input) {
    size_t len = strlen(input);
    if (*input == '\0' || (*input == '0' && len == 1)) {
        *output = INCLUDES_BREAKFAST_FALSE;
        return 0;
    } else if (*input == '1' && len == 1) {
        *output = INCLUDES_BREAKFAST_TRUE;
        return 0;
    }

    char lower_case[INCLUDES_BREAKFAST_STRLEN_FALSE + 1];
    for (size_t i = 0; i < len; ++i)
        lower_case[i] = tolower(input[i]);
    lower_case[len] = '\0';
    
    if(!strcmp(lower_case, "f") || !strcmp(lower_case, "false")) {
        *output = INCLUDES_BREAKFAST_FALSE;
        return 0;
    } else if (!strcmp(lower_case, "t") || !strcmp(lower_case, "true")) {
        *output = INCLUDES_BREAKFAST_TRUE;
        return 0;
    }
    return 1;
}

void includes_breakfast_sprintf(char *output, includes_breakfast_t breakfast) {
    const char *source = "";

    switch (breakfast) {
        case INCLUDES_BREAKFAST_FALSE:
            source = "false";
            break;
        case INCLUDES_BREAKFAST_TRUE:
            source = "true";
            break;
    }

    strcpy(output, source);
}
