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
 * @file  account_status.c
 * @brief Implementation of methods in include/types/account_status.h
 */

#include <ctype.h>
#include <string.h>

#include "types/account_status.h"

/** @brief Value of `strlen("inactive")`. */
#define ACCOUNT_STATUS_STRLEN_INACTIVE 8

int account_status_from_string(account_status_t *output, const char *input) {
    size_t len = strlen(input);
    if (len > ACCOUNT_STATUS_STRLEN_INACTIVE)
        return 1; /* Too long for "inactive" */

    char lower_case[ACCOUNT_STATUS_STRLEN_INACTIVE + 1];
    for (size_t i = 0; i < len; ++i)
        lower_case[i] = tolower(input[i]);
    lower_case[len] = '\0';

    if (strcmp(lower_case, "inactive") == 0) {
        *output = ACCOUNT_STATUS_INACTIVE;
        return 0;
    } else if (strcmp(lower_case, "active") == 0) {
        *output = ACCOUNT_STATUS_ACTIVE;
        return 0;
    } else {
        return 1;
    }
}
