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
 * @file    email.h
 * @brief   The email of a ::user_t.
 *
 * @anchor email_examples
 * ### Examples
 *
 * The following example shows what valid / invalid emails consist of:
 *
 * ```c
 * #include <inttypes.h>
 * #include <stdio.h>
 *
 * #include "types/email.h"
 *
 * int main(void) {
 *     const char *emails[8] = {
 *         "a104348@gmail.com", // Valid email
 *         "@email.com",        // No username
 *         "domain.pt",         // No at sign
 *         "john.email.pt",     // No at sign
 *         "john@.pt",          // No domain
 *         "john@email",        // No TLD
 *         "john@email.a",      // Too short of a TLD
 *         "john@email.com"     // Valid email
 *     };
 *
 *     for (size_t i = 0; i < 8; ++i) {
 *         int valid = email_validate_string_const(emails[i]);
 *         printf("\"%s\" is %s email\n", emails[i], valid ? "an invalid" : "a valid");
 *     }
 *
 *     return 0;
 * }
 * ```
 */

#ifndef EMAIL_H
#define EMAIL_H

#include <stdint.h>

/**
 * @brief Verifies if a **MODIFIABLE** string is a valid email.
 *
 * @param input  String to validate, that won't be modified. Must be in the format
 *               `"user@domain.tld"`.
 *
 * @retval 0 Valid email.
 * @retval 1 Validation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref email_examples). The example shows
 * ::email_validate_string_const, but it serves the same purpose as this method, just for
 * `const char *`.
 */
int email_validate_string(char *input);

/**
 * @brief Verifies if a string is a valid email.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param input  String to validate. Must be in the format `"user@domain.tld"`.
 *
 * @retval 0 Valid email.
 * @retval 1 Validation or allocation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref email_examples).
 */
int email_validate_string_const(const char *input);

#endif
