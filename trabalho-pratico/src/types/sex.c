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
 * @file  sex.c
 * @brief Implementation of methods in include/types/sex.h
 */

#include <string.h>

#include "types/sex.h"

int sex_from_string(sex_t *output, const char *input) {
    if (strcmp(input, "M") == 0) {
        *output = SEX_M;
        return 0;
    } else if (strcmp(input, "F") == 0) {
        *output = SEX_F;
        return 0;
    } else {
        return 1;
    }
}

void sex_sprintf(char *output, sex_t sex) {
    if (sex == SEX_M) {
        *output = 'M';
    } else {
        *output = 'F';
    }
    output[1] = '\0';
}
