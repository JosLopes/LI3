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
 * @file    daytime.h
 * @brief   A time, containing hours, minutes and seconds.
 *
 * @anchor daytime_examples
 * ### Examples
 *
 * The following example shows how to parse and print times. You can also see how string with times
 * must be structured in order to be parsed.
 *
 * ```
 * const char *times[11] = {
 *     "00:44:24", // Date during a sleepless programming session
 *
 *     "0:12:45",  // One digit missing
 *     "00:-1:12", // Invalid digit
 *
 *     "00:00:00", // Valid time
 *     "23:59:59", // Valid time
 *     "24:59:59", // Out of range hour
 *     "23:60:59", // Out of range minute
 *     "23:59:60", // Out of range second
 *
 *     "00:00:00:00", // Too many data points
 *     "00:00",       // Too few data points
 *     "",            // What?
 * };
 *
 * for (int i = 0; i < 11; ++i) {
 *     daytime_t time;
 *     int success = daytime_from_string_const(&time, times[i]);
 *
 *     if (success) {
 *         fprintf(stderr, "Failed to parse time \"%s\".\n", times[i]);
 *     } else {
 *         char str[DAYTIME_SPRINTF_MIN_BUFFER_SIZE];
 *         daytime_sprintf(str, time);
 *
 *         printf("%s was parsed successfully.\n", str);
 *     }
 * }
 * ```
 *
 * Setters of `daytime_t` can fail! For example, `daytime_set_seconds(&time, 60)` will do nothing
 * and return `1`, because `60` is not an accepted value for seconds. The same applies to
 * ::daytime_from_values.
 */

#ifndef DAYTIME_H
#define DAYTIME_H

#include <stdint.h>

/**
 * @brief A time containing hours, minutes and seconds.
 */
typedef int32_t daytime_t;

/**
 * @brief Creates a time from @p hours, @p minute and @p second.
 *
 * @param output  Where the created time will be placed.
 * @param hours   Hours of the time (between 0 and 23)
 * @param minutes Minutes of the time (between 0 and 59).
 * @param seconds Seconds of the time (between 0 and 59).
 *
 * @retval 0 Success
 * @retval 1 Failure due to out of range values.
 */
int daytime_from_values(daytime_t *output, uint8_t hours, uint8_t minutes, uint8_t seconds);

/**
 * @brief Parses a **MODIFIABLE** string containing a time.
 *
 * @param output Where the parsed time is placed. Won't be modified on failure.
 * @param input  String to parse, that will be modified. Must be in the format `"HH:MM:SS"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref daytime_examples). The example shows
 * ::daytime_from_string_const, but it serves the same purpose as this method, just for
 * `const char *`.
 */
int daytime_from_string(daytime_t *output, char *input);

/**
 * @brief Parses a string containing a time.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param output Where the parsed time is placed. Won't be modified on failure.
 * @param input  String to parse. Must be in the format `"HH:MM:SS"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing or `malloc` failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref daytime_examples).
 */
int daytime_from_string_const(daytime_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write a time using
 *        ::daytime_sprintf.
 * @details
 *
 * | H | H | : | M | M | : | S | S | \0 |
 * | - | - | - | - | - | - | - | - | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9  |
 */
#define DAYTIME_SPRINTF_MIN_BUFFER_SIZE 9

/**
 * @brief Prints a time using `sprintf` in the format `HH:MM:SS`.
 *
 * @param output Where to print the time to. Must be at least ::DAYTIME_SPRINTF_MIN_BUFFER_SIZE
 *               long.
 * @param time   Time to be printed.
 *
 * #### Examples
 * See [the header file's documentation](@ref daytime_examples).
 */
void daytime_sprintf(char *output, daytime_t time);

/**
 * @brief Calculates the difference (in seconds) between two times.
 *
 * @param a Time from which @p b is subtracted from.
 * @param b Time subtracted from @p a.
 *
 * @return The difference `a - b` in seconds.
 */
uint32_t daytime_diff(daytime_t a, daytime_t b);

/**
 * @brief Gets the hours from a time.
 * @param time Time to get the hours from.
 * @return Hour in the time.
 */
uint8_t daytime_get_hours(daytime_t time);

/**
 * @brief Sets the hours in a time.
 *
 * @param time  Time to be modified.
 * @param hours Value of the hours.
 *
 * @retval 0 Success
 * @retval 1 Failure due to out of range value.
 */
int daytime_set_hours(daytime_t *time, uint8_t hours);

/**
 * @brief Gets the minutes from a time.
 * @param time Time to get the minutes from.
 * @return Minutes in the time.
 */
uint8_t daytime_get_minutes(daytime_t time);

/**
 * @brief Sets the minutes in a time.
 *
 * @param time    Time to be modified.
 * @param minutes Value of the minutes.
 *
 * @retval 0 Success
 * @retval 1 Failure due to out of range value.
 */
int daytime_set_minutes(daytime_t *time, uint8_t minutes);

/**
 * @brief Gets the seconds from a time.
 * @param time Time to get the seconds from.
 * @return Seconds in the time.
 */
uint8_t daytime_get_seconds(daytime_t time);

/**
 * @brief Sets the seconds in a time.
 *
 * @param time   Time to be modified.
 * @param seconds Value of the seconds.
 *
 * @retval 0 Success
 * @retval 1 Failure due to out of range value.
 */
int daytime_set_seconds(daytime_t *time, uint8_t seconds);

#endif
