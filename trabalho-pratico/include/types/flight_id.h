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
 * @file  flight_id.h
 * @brief Identifier of a ::flight_t.
 */

#ifndef FLIGHT_ID_H
#define FLIGHT_ID_H

#include <inttypes.h>

/** @brief Identifier of a ::flight_t. */
typedef uint32_t flight_id_t;

/**
 * @brief Parses a flight's identifier.
 *
 * @param output Where the parsed identifier will be placed (only on success).
 * @param input  Identifier as a string (must be comprised of 10 decimal digits).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure (empty string)
 * @retval 2 Parsing failure (not 10 digits, case which should technically be supported).
 */
int flight_id_from_string(flight_id_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write the
 *        identifier of a flight.
 * @details
 *
 * | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0  | \0 |
 * | - | - | - | - | - | - | - | - | - | -- | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
 */
#define FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE 11

/**
 * @brief Prints the identifier of a flight using `sprintf`.
 *
 * @param output Where to print the flight identifier to. Must be at least
 *               ::FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE long.
 * @param id     Flight identifier.
 */
void flight_id_sprintf(char *output, flight_id_t id);

#endif
