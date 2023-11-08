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
 * @file  includes_breakfast.h
 * @brief Includes breakfast of a ::reservation_t.
 */

#ifndef INCLUDES_BREAKFAST_H
#define INCLUDES_BREAKFAST_H

/**
 * @brief   Indicates if breakfast is included for a ::reservation_t.
 * @details For the values `INCLUDES_BREAKFAST_0`, `INCLUDES_BREAKFAST_NO_INPUT`,
 *          `INCLUDES_BREAKFAST_F` and `INCLUDES_BREAKFAST_FALSE` a breakfast is not included, for
 *          every other value the reservation includes breakfast.
 */
typedef enum includes_breakfast {
    INCLUDES_BREAKFAST_0,
    INCLUDES_BREAKFAST_1,
    INCLUDES_BREAKFAST_NO_INPUT,
    INCLUDES_BREAKFAST_F,
    INCLUDES_BREAKFAST_FALSE,
    INCLUDES_BREAKFAST_T,
    INCLUDES_BREAKFAST_TRUE
} includes_breakfast_t;

/**
 * @brief Parses a reservation's inclusion of breakfast.
 *
 * @param output Where the parsed value will be placed (only on success).
 * @param input  Input (`"0"`,`"1"`, NULL (no input), `"f"`, `"false"`, `"t"` or `"true"`).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure
 */
int includes_breakfast_from_string(includes_breakfast_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write an "includes
 *        breakfast" field to a buffer using ::includes_breakfast_sprintf.
 * @details
 *
 * | F | A | L | S | E | \0 |
 * | - | - | - | - | - | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6  |
 */
#define INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE 6

/**
 * @brief Prints an "includes breakfast" field using `sprintf`.
 *
 * @param output    Where to print the airport code to. Must be at least
 *                  ::INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE long.
 * @param breakfast "Includes breakfast" field.
 */
void includes_breakfast_sprintf(char *output, includes_breakfast_t breakfast);

#endif
