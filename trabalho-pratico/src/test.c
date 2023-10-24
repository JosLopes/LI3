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
 * @file test.c
 * @brief Contains the entry point to the program.
 */

#include <inttypes.h>
#include <stdio.h>

#include "types/email.h"

int main(void) {
    const char *emails[8] = {
        "a104348@gmail.com", /* Valid email */
        "@email.com",        /* No username */
        "domain.pt",         /* No at sign */
        "john.email.pt",     /* No at sign */
        "john@.pt",          /* No domain */
        "john@email",        /* No TLD */
        "john@email.a",      /* Too short of a TLD */
        "john@email.com"     /* Valid email */
    };

    for (size_t i = 0; i < 8; ++i) {
        int valid = email_validate_string_const(emails[i]);
        printf("\"%s\" is %s email\n", emails[i], valid ? "an invalid" : "a valid");
    }

    return 0;
}
