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
 * @file  airport_code.h
 * @brief Airport code in a ::flight_t.
 *
 * @anchor airport_examples
 * ### Examples
 *
 * The following example shows how to validate airport codes and how to print them back to strings.
 * It's also shown that all airport codes are stored in upper-case.
 *
 * ```c
 * const char *string_codes[6] = {
 *     "",     // Too short
 *     "O",    // Too short
 *     "OP",   // Too short
 *     "OPO",  // Just right
 *     "oPo",  // Everything should be converted to upper-case
 *     "OPOR", // Too long
 * };
 *
 * for (int i = 0; i < 6; ++i) {
 *     airport_code_t parsed;
 *
 *     if (!airport_code_from_string(&parsed, string_codes[i])) {
 *         char back_to_string[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
 *         airport_code_sprintf(back_to_string, parsed);
 *         printf("Parsed code: \"%s\"\n", back_to_string);
 *
 *     } else {
 *         fprintf(stderr, "Failed to parse airport code \"%s\"!\n", string_codes[i]);
 *     }
 * }
 * ```
 */

#ifndef AIRPORT_CODE_H
#define AIRPORT_CODE_H

#include <stdint.h>

/**
 * @brief An airport code of a ::flight_t.
 */
typedef uint32_t airport_code_t;

/**
 * @brief Creates (and validates) a airport code from a string.
 *
 * @param output Where to place the parsed airport code (not modified on failure).
 * @param input  String to parse.
 *
 * @retval 0 Valid airport code
 * @retval 1 Invalid airport code
 */
int airport_code_from_string(airport_code_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write an airport
 *        code to a buffer using ::airport_code_sprintf.
 * @details
 *
 * | O | P | O | \0 |
 * | - | - | - | -- |
 * | 1 | 2 | 3 | 4  |
 */
#define AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE 4

/**
 * @brief Prints a airport code using `sprintf`.
 *
 * @param output  Where to print the airport code to. Must be at least
 *                ::AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE long.
 * @param airport Country code to be printed.
 */
void airport_code_sprintf(char *output, airport_code_t airport);

#endif
