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
 * @file  sex.h
 * @brief Sex of a ::user_t.
 */

#ifndef SEX_H
#define SEX_H

/** @brief The sex of a ::user_t. */
typedef enum {
    SEX_F, /**< @brief Female */
    SEX_M  /**< @brief Male */
} sex_t;

/**
 * @brief Parses a user's sex.
 *
 * @param output Where the parsed sex will be placed (only on success).
 * @param input  Input (`"M"` or `"F"`).
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure
 */
int sex_from_string(sex_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write a sex tag
 *        to a buffer using ::sex_sprintf.
 * @details
 *
 * | F | \0 |
 * | - | -- |
 * | 1 | 2  |
 */
#define SEX_SPRINTF_MIN_BUFFER_SIZE 2

/**
 * @brief Prints a sex tag to a buffer.
 *
 * @param output Where to print the sex tag to. Must be at least ::SEX_SPRINTF_MIN_BUFFER_SIZE
 *               characters long.
 * @param sex    Sex to be printed.
 */
void sex_sprintf(char *output, sex_t sex);

#endif
