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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <stdio.h>
#include <stdlib.h>

#include "types/airport_code.h"

/**
 * @brief The entry point to the test program.
 * @details Test for airport codes.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    const char *string_codes[6] = {
        "",     /* Too short */
        "O",    /* Too short */
        "OP",   /* Too short */
        "OPO",  /* Just right */
        "oPo",  /* Everything should be converted to upper-case */
        "OPOR", /* Too long */
    };

    for (int i = 0; i < 6; ++i) {
        airport_code_t parsed;

        if (!airport_code_from_string(&parsed, string_codes[i])) {
            char back_to_string[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
            airport_code_sprintf(back_to_string, parsed);
            printf("Parsed code: \"%s\"\n", back_to_string);

        } else {
            fprintf(stderr, "Failed to parse airport code \"%s\"!\n", string_codes[i]);
        }
    }

    return 0;
}
