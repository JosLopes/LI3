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
 * @file  reservation_id.h
 * @brief Identifier of a ::reservation_t.
 */

#ifndef RESERVATION_ID_H
#define RESERVATION_ID_H

#include <inttypes.h>

/** @brief Identifier of a ::reservation_t. */
typedef uint32_t reservation_id_t;

/**
 * @brief Parses a reservation's identifier.
 *
 * @param output Where the parsed identifier will be placed (only on success).
 * @param input  Identifier as a string (must be in format `BookXXXXXXXXXX`, where each `X` is a
 *               decimal digit).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure (empty string)
 * @retval 2 Parsing failure (non-`BookXXXXXXXXXX` identifier that should technically be supported).
 */
int reservation_id_from_string(reservation_id_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write the
 *        identifier of a reservation.
 * @details
 *
 * | B | o | o | k | 0 | 0 | 0 | 0 | 0 | 0  | 0  | 0  | 0  | 0  | \0 |
 * | - | - | - | - | - | - | - | - | - | -- | -- | -- | -- | -- | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
 */
#define RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE 15

/**
 * @brief Prints the identifier of a reservation using `sprintf`.
 *
 * @param output Where to print the reservation identifier to. Must be at least
 *               ::RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE characters long.
 * @param id     Reservation identifier.
 */
void reservation_id_sprintf(char *output, reservation_id_t id);

#endif
