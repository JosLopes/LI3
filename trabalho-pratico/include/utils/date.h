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
 * @file  date.h
 * @brief A date containing a year, a month and a day.
 *
 * @anchor date_examples
 * ### Examples
 *
 * The following example shows how dates can be parsed, what configures a valid date, how to modify
 * a date using getters and setters, and how to print a date to a string.
 *
 * ```
 * #include <stdio.h>
 * #include <stdlib.h>
 *
 * #include "utils/date.h"
 *
 * int main(void) {
 *     const char *birthdays[11] = {
 *         "2004/10/16", // Valid date
 *
 *         "2002/7/12",   // One digit missing
 *         "13000/04/12", // One extra digit
 *         "-203/06/01",  // Invalid digit
 *         "-203/1-/01",  // Invalid digit
 *
 *         "2022/04/31", // Valid date
 *         "2022/05/32", // Out-of-range day
 *         "1333/00/12", // Out-of-range month
 *         "1333/13/12", // Out-of-range month
 *
 *         "2023/11/21/11", // Too many data points
 *         "2023/11"        // Too few data points
 *     };
 *
 *     for (int i = 0; i < 11; ++i) {
 *         date_t date;
 *         int success = date_from_string_const(&date, birthdays[i]);
 *
 *         if (success) {
 *             fprintf(stderr, "Failed to parse date \"%s\".\n", birthdays[i]);
 *         } else {
 *             // This may fail for large years (max year is 9999). This example doesn't check for
 *             // that error.
 *             date_set_year(&date, date_get_year(date) + 10);
 *
 *             char str[DATE_SPRINTF_MIN_BUFFER_SIZE];
 *             date_sprintf(str, date);
 *             printf("Your 10th birthday was / will be in %s\n", str);
 *         }
 *     }
 *
 *    return 0;
 * }
 * ```
 * Setters of `date_t` can fail! For example, `date_set_month(&date, 13)` will do nothing
 * and return `1`, because `13` is not an accepted value for a month. The same applies to
 * ::date_from_values.
 */

#ifndef DATE_H
#define DATE_H

#include <stdint.h>

/** @brief A date containing a year, a month a day. */
typedef uint32_t date_t;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    /** @brief Current system date (`2023/10/01`). */
    #define DATE_CURRENT 0x07e70a01
#else
    /** @brief Current system date (`2023/10/01`). */
    #define DATE_CURRENT 0x010a07e7
#endif

/**
 * @brief Creates a date from a @p year, a @p month and a @p day.
 *
 * @param output Where the created date will be placed.
 * @param year   Year of the date (a number of four decimal digits).
 * @param month  Month of the date (between 1 and 12).
 * @param day    Day of the date (between 1 and 31, independently of @p month).
 *
 * @retval 0 Success
 * @retval 1 Failure due to out of range values.
 */
int date_from_values(date_t *output, uint16_t year, uint8_t month, uint8_t day);

/**
 * @brief Parses a **MODIFIABLE** string containing a date.
 *
 * @param output Where the parsed date is placed. Won't be modified on failure.
 * @param input  String to parse, that will be modified during parsing, but then restored to its
 *               original form. Must be in the format `"YYYY/MM/DD"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_examples). The example shows
 * ::date_from_string_const, but it serves the same purpose as this method, just for `const char *`.
 */
int date_from_string(date_t *output, char *input);

/**
 * @brief   Parses a string containing a date.
 * @details The current implementation copies the provided string to a temporary buffer. Keep that
 *          in mind for performance reasons.
 *
 * @param output Where the parsed date is placed. Won't be modified on failure.
 * @param input  String to parse. Must be in the format `"YYYY/MM/DD"`.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing or `malloc` failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_examples).
 */
int date_from_string_const(date_t *output, const char *input);

/**
 * @brief The minimum number of characters (including null terminator) needed to write a date using
 *        ::date_sprintf.
 * @details
 *
 * | Y | Y | Y | Y | / | M | M | / | D | D  | \0 |
 * | - | - | - | - | - | - | - | - | - | -- | -- |
 * | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
 */
#define DATE_SPRINTF_MIN_BUFFER_SIZE 11

/**
 * @brief Prints a date using `sprintf` in the format `YYYY/MM/DD`.
 *
 * @param output Where to print the date to. Must be at least ::DATE_SPRINTF_MIN_BUFFER_SIZE
 *               characters long.
 * @param date   Date to be printed.
 *
 * #### Examples
 * See [the header file's documentation](@ref date_examples).
 */
void date_sprintf(char *output, date_t date);

/**
 * @brief   Calculates the difference (in days) between two dates.
 * @details This formula assumes all months have `31` days, and all years `12 * 31` days. This
 *          shouldn't matter anyway, as project requirements specify that datasets contain only
 *          dates from the same month.
 *
 * @param a Date from which @p b is subtracted from.
 * @param b Date subtracted from @p a.
 *
 * @return The difference `a - b` in days.
 */
int64_t date_diff(date_t a, date_t b);

/**
 * @brief  Gets the year from a date.
 * @param  date Date to get the year from.
 * @return Year in the date.
 */
uint16_t date_get_year(date_t date);

/**
 * @brief Sets the year in a date.
 *
 * @param date Date to be modified.
 * @param year Value of the year.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to out of range value.
 */
int date_set_year(date_t *date, uint16_t year);

/**
 * @brief  Gets the month from a date.
 * @param  date Date to get the month from.
 * @return Month in the date.
 */
uint8_t date_get_month(date_t date);

/**
 * @brief Sets the month in a date.
 *
 * @param date  Date to be modified.
 * @param month Value of the month.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to out of range value.
 */
int date_set_month(date_t *date, uint8_t month);

/**
 * @brief  Gets the day from a date.
 * @param  date Date to get the day from.
 * @return Day in the date.
 */
uint8_t date_get_day(date_t date);

/**
 * @brief Sets the day in a date.
 *
 * @param date Date to be modified.
 * @param day  Value of the day.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to out of range value.
 */
int date_set_day(date_t *date, uint8_t day);

/**
 * @brief   Generates an integer made of a date without its day.
 * @details Useful for referring to months in hash table keys.
 * @return  An integer for a date without a day (only month and year).
 */
uint32_t date_generate_dayless(date_t date);

/**
 * @brief   Generates an integer made of a date without its month and day.
 * @details Useful for referring to years in hash table keys.
 * @return  An integer for a date without months and days (only years).
 */
uint32_t date_generate_monthless(date_t date);

#endif
