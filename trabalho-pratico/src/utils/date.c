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
 * @file  date.c
 * @brief Implementation of methods in include/utils/date.h
 *
 * ### Examples
 * See [the header file's documentation](@ref date_examples).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/date.h"
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/int_utils.h"
#include "utils/string_utils.h"

/**
 * @brief Grammar for parsing dates.
 * @details Shall not be modified apart from its creation.
 *
 * TODO - ask professors if we can use global variables
 */
fixed_n_delimiter_parser_grammar_t *date_grammar = NULL;

/** @brief Date with `union`, to easily extract fields from a date integer. */
typedef union {
    date_t date;

    struct {
        uint16_t year;
        uint8_t  month, day;
    } fields;
} date_union_helper_t;

/** @brief The minimum value (inclusive) that a year in a date may take. */
#define DATE_YEAR_MIN  1
/** @brief The maximum value (inclusive) that a year in a date may take. */
#define DATE_YEAR_MAX  9999
/** @brief The minimum value (inclusive) that a month in a date may take. */
#define DATE_MONTH_MIN 1
/** @brief The maximum value (inclusive) that a month in a date may take. */
#define DATE_MONTH_MAX 12
/** @brief The minimum value (inclusive) that a day in a date may take. */
#define DATE_DAY_MIN   1
/** @brief The maximum value (inclusive) that a day in a date may take. */
#define DATE_DAY_MAX   31

int date_from_values(date_t *output, uint16_t year, uint8_t month, uint8_t day) {
    date_union_helper_t date_union;

    if (year < DATE_YEAR_MIN || year > DATE_YEAR_MAX || month < DATE_MONTH_MIN ||
        month > DATE_MONTH_MAX || day < DATE_DAY_MIN || day > DATE_DAY_MAX) {

        return 1;
    }

    date_union.fields.year  = year;
    date_union.fields.month = month;
    date_union.fields.day   = day;

    *output = date_union.date;
    return 0;
}

/**
 * @brief Auxiliary method for ::date_from_string. Parses any of the integers in a date.
 */
int __date_from_string_parse_field(void *date_data, char *token, size_t ntoken) {
    date_union_helper_t *date = (date_union_helper_t *) date_data;

    uint64_t mins[3]    = {DATE_YEAR_MIN, DATE_MONTH_MIN, DATE_DAY_MIN};
    uint64_t maxs[3]    = {DATE_YEAR_MAX, DATE_MONTH_MAX, DATE_DAY_MAX};
    uint64_t lengths[3] = {4, 2, 2};

    size_t token_length = strlen(token);
    if (token_length != lengths[ntoken])
        return 1;

    uint64_t parsed           = 0;
    int      int_parse_result = int_utils_parse_positive(&parsed, token);
    if (int_parse_result) {
        return 1;
    }

    if (parsed < mins[ntoken] || parsed > maxs[ntoken]) /* Bounds checking */
        return 1;

    switch (ntoken) {
        case 0:
            date->fields.year = parsed;
            break;

        case 1:
            date->fields.month = parsed;
            break;

        case 2:
            date->fields.day = parsed;
            break;
    }
    return 0;
}

/**
 * @brief Method used with `atexit` to free the grammar created by ::date_from_string.
 */
void __date_from_string_free_grammar(void) {
    free(date_grammar);
}

int date_from_string(date_t *output, char *input) {
    /* Create grammar if needed */
    if (!date_grammar) {
        fixed_n_delimiter_parser_iter_callback_t callbacks[3] = {__date_from_string_parse_field,
                                                                 __date_from_string_parse_field,
                                                                 __date_from_string_parse_field};

        /* NOTE - Make thread-safe if multithreading gets implemented */
        date_grammar = fixed_n_delimiter_parser_grammar_new('/', 3, callbacks);
        if (!date_grammar) {
            return 1;
        }
        atexit(__date_from_string_free_grammar);
    }

    /* Actually parse date */
    date_union_helper_t tmp_date;
    int retval = fixed_n_delimiter_parser_parse_string(input, date_grammar, &tmp_date);
    if (retval) {
        return retval;
    } else {
        *output = tmp_date.date;
        return 0;
    }
}

int date_from_string_const(date_t *output, const char *input) {
    char *buffer = string_duplicate(input);
    if (!buffer)
        return 1;

    int retval = date_from_string(output, buffer);

    free(buffer);
    return retval;
}

int date_sprintf(char *output, date_t date) {
    date_union_helper_t date_union = {.date = date};

    return sprintf(output,
                   "%04d/%02d/%02d",
                   date_union.fields.year,
                   date_union.fields.month,
                   date_union.fields.day);
}

uint64_t date_diff(date_t a, date_t b) {
    date_union_helper_t a_union = {.date = a};
    date_union_helper_t b_union = {.date = b};

    uint64_t a_days =
        (uint64_t) a_union.fields.year * 12 * 31 + a_union.fields.month * 31 + a_union.fields.day;
    uint64_t b_days =
        (uint64_t) b_union.fields.year * 12 * 31 + b_union.fields.month * 31 + b_union.fields.day;

    return a_days - b_days;
}

/**
 * @brief Helper macro for defining getters.
 * @param property Property to get in ::date_union_helper_t.fields.
 */
#define DATE_GETTER_FUNCTION_BODY(property)                                                        \
    date_union_helper_t date_union = {.date = date};                                               \
    return date_union.fields.property;

/**
 * @brief Helper macro for defining setters.
 * @param property Property to set in ::date_union_helper_t.fields. Name must match the name of the
 *                 argument in the setter method.
 * @param lower_bound Minimum value (inclusive) that @p property can take.
 * @param upper_bound Maximum value (inclusive) that @p property can take.
 */
#define DATE_SETTER_FUNCTION_BODY(property, lower_bound, upper_bound)                              \
    if (property < lower_bound || property > upper_bound) {                                        \
        return 1;                                                                                  \
    }                                                                                              \
                                                                                                   \
    date_union_helper_t date_union = {.date = *date};                                              \
    date_union.fields.property     = property;                                                     \
    *date                          = date_union.date;                                              \
    return 0;

uint16_t date_get_year(date_t date) {
    DATE_GETTER_FUNCTION_BODY(year);
}

int date_set_year(date_t *date, uint16_t year) {
    DATE_SETTER_FUNCTION_BODY(year, DATE_YEAR_MIN, DATE_YEAR_MAX);
}

uint8_t date_get_month(date_t date) {
    DATE_GETTER_FUNCTION_BODY(month);
}

int date_set_month(date_t *date, uint8_t month) {
    DATE_SETTER_FUNCTION_BODY(month, DATE_MONTH_MIN, DATE_MONTH_MAX);
}

uint8_t date_get_day(date_t date) {
    DATE_GETTER_FUNCTION_BODY(day);
}

int date_set_day(date_t *date, uint8_t day) {
    DATE_SETTER_FUNCTION_BODY(day, DATE_DAY_MIN, DATE_DAY_MAX);
}
