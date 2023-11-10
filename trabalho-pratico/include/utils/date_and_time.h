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
 * @file    date_and_time.h
 * @brief   A structure containing a ::date_t and and ::daytime_t.
 *
 * @anchor date_and_time_examples
 * ### Examples
 *
 * The following example shows how to parse and print timed dates. For more information about
 * date parsing, see [this](@ref date_examples), and [this](@ref daytime_examples) for the parsing
 * of times.
 *
 * ```
 * const char *date_and_times[2] = {
 *     "2023/11/11 23:59:59",  // Due date for this project (we're screwed)
 *     "2023/11/11 23:59:59 ", // Too many spaces
 * };
 *
 * for (int i = 0; i < 2; ++i) {
 *     date_and_time_t date_and_time;
       int success = date_and_time_from_string_const(&date_and_time, date_and_times[i]);

        if (success) {
            fprintf(stderr, "Failed to parse timed date \"%s\".\n", date_and_times[i]);
        } else {
            char str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
            date_and_time_sprintf(str, date_and_time);

            printf("%s was parsed successfully.\n", str);
        }
    }

    return 0;
 * ```
 */

#ifndef DATE_AND_TIME_H
#define DATE_AND_TIME_H

#include <stdint.h>

#include "utils/date.h"
#include "utils/daytime.h"

/**
 * @brief A type containing a ::date_t and a ::daytime_t.
 */
typedef int64_t date_and_time_t;

/**
 * @brief Creates a ::date_and_time_t from its @p date and @p time values.
 *
 * @param output Where the created date and time will be placed.
 * @param date   Date in the timed date.
 * @param time   Time in the timed date.
 */
void date_and_time_from_values(date_and_time_t *output, date_t date, daytime_t time);

/**
 * @brief Parses a **MODIFIABLE** string containing a date and a time.
 *
 * @param output Where the parsed timed date is placed. Won't be modified on failure.
 * @param input  String to parse, that won't be modified. Must be in the format
 *               `"YYYY/MM/DD HH:MM:SS"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_and_time_examples). The example shows
 * ::date_and_time_from_string_const, but it serves the same purpose as this method, just for
 * `const char *`.
 */
int date_and_time_from_string(date_and_time_t *output, char *input);

/**
 * @brief Parses a string containing a date and a time.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param output Where the parsed timed date is placed. Won't be modified on failure.
 * @param input  String to parse. Must be in the format `"YYYY/MM/DD HH:MM:SS"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing or `malloc` failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_and_time_examples).
 */
int date_and_time_from_string_const(date_and_time_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write a timed date
 *        using ::date_and_time_sprintf.
 * @details
 *
 * | Y | Y | Y | Y | / | M | M | / | D | D  |    | H  | H  | :  | M  | M  | :  | S  | S  | \0 |
 * | - | - | - | - | - | - | - | - | - | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 |
 */
#define DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE                                                      \
    (DATE_SPRINTF_MIN_BUFFER_SIZE + DAYTIME_SPRINTF_MIN_BUFFER_SIZE)

/**
 * @brief Prints a timed date using `sprintf` in the format `YYYY/MM/DD HH:MM:SS`.
 *
 * @param output Where to print the timed date to. Must be at least
 *               ::DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE long.
 * @param date   Timed date to be printed.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_and_time_examples).
 */
void date_and_time_sprintf(char *output, date_and_time_t date_and_time);

/**
 * @brief Calculates the difference (in seconds) between two timed dates.
 * @details This formula assumes all months have `31` days, and all years `12 * 31` days. This
 *          shouldn't matter anyway, as project requirements specify that datasets contain only
 *          dates from the same month.
 *
 * @param a Timed date from which @p b is subtracted from.
 * @param b Timed date subtracted from @p a.
 *
 * @return The difference `a - b` in seconds.
 */
int64_t date_and_time_diff(date_and_time_t a, date_and_time_t b);

/**
 * @brief Gets the date from a timed date.
 * @param date_and_time Timed date to get the date from.
 * @return Date in the timed date.
 */
date_t date_and_time_get_date(date_and_time_t date_and_time);

/**
 * @brief Sets the date in a timed date.
 * @param date_and_time Timed date to be modified.
 * @param date          Value of the date.
 */
void date_and_time_set_date(date_and_time_t *date_and_time, date_t date);

/**
 * @brief Gets the time from a timed date.
 * @param date_and_time Timed date to get the time from.
 * @return Time in the timed date.
 */
daytime_t date_and_time_get_time(date_and_time_t date_and_time);

/**
 * @brief Sets the time in a timed date.
 * @param date_and_time Timed date to be modified.
 * @param time          Value of the time.
 */
void date_and_time_set_time(date_and_time_t *date_and_time, daytime_t time);

#endif
