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
 * @file     country_code.h
 * @brief    Country code in a ::user_t.
 *
 * @anchor date_examples
 * ### Examples
 *
 * The following example shows how to validate country codes and how to print them back to strings.
 *
 * ```c
 * const char *string_codes[4] = {
 *     "",    // Too short
 *     "P",   // Too short
 *     "PT",  // Just right
 *     "POR", // Too long
 * };
 *
 * for (int i = 0; i < 4; ++i) {
 *     country_code_t parsed;
 *
 *     if (!country_code_from_string(&parsed, string_codes[i])) {
 *         char back_to_string[COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE];
 *         country_code_sprintf(back_to_string, parsed);
 *         printf("Parsed code: \"%s\"\n", back_to_string);
 *
 *     } else {
 *         fprintf(stderr, "Failed to parse country code \"%s\"!\n", string_codes[i]);
 *     }
 * }
 * ```
 */

#ifndef COUNTRY_CODE_H
#define COUNTRY_CODE_H

#include <stdint.h>

/**
 * @brief A country code of a ::user_t.
 */
typedef uint16_t country_code_t;

/**
 * @brief Creates (and validates) a country code from a string.
 *
 * @param output Where to place the parsed country code (not modified on failure).
 * @param input  String to parse.
 *
 * @retval 0 Valid country code
 * @retval 1 Invalid country code
 */
int country_code_from_string(country_code_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write a country
 *        code to a buffer using ::country_code_sprintf.
 * @details
 *
 * | P | T | \0 |
 * | - | - | -- |
 * | 1 | 2 | 3  |
 */
#define COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE 3

/**
 * @brief Prints a country code using `sprintf`.
 *
 * @param output  Where to print the country code to. Must be at least
 *                ::COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE long.
 * @param country Country code to be printed.
 */
void country_code_sprintf(char *output, country_code_t country);

#endif
