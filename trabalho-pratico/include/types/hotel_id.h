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
 * @file  hotel_id.h
 * @brief Identifier of a hotel in a ::reservation_t.
 */

#ifndef HOTEL_ID_H
#define HOTEL_ID_H

#include <inttypes.h>

/** @brief Identifier of a hotel in a ::reservation_t. */
typedef uint16_t hotel_id_t;

/**
 * @brief Parses the identifier of a reservation's hotel.
 *
 * @param output Where the parsed identifier will be placed (only on success).
 * @param input  Identifier as a string (must be in format `HTLXXXXX`, where each `X` is a
 *               decimal digit).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure (empty string)
 * @retval 2 Parsing failure (non-`HTLXXXXX` identifier that should technically be supported).
 */
int hotel_id_from_string(hotel_id_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write the
 *        identifier of a reservation's hotel.
 * @details
 *
 * | H | T | L | 0 | 0 | 0 | 0 | 0 | \0 |
 * | - | - | - | - | - | - | - | - | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9  |
 */
#define HOTEL_ID_SPRINTF_MIN_BUFFER_SIZE 9

/**
 * @brief Prints the identifier of a hotel in a reservation using `sprintf`.
 *
 * @param output Where to print the hotel identifier to. Must be at least
 *               ::HOTEL_ID_SPRINTF_MIN_BUFFER_SIZE long.
 * @param id     Hotel identifier.
 */
void hotel_id_sprintf(char *output, hotel_id_t id);

#endif
