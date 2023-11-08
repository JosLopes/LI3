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
 * @file  date_and_time.c
 * @brief Implementation of methods in include/utils/date_and_time.h
 *
 * ### Examples
 * See [the header file's documentation](@ref date_and_time_examples).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/date.h"
#include "utils/date_and_time.h"
#include "utils/daytime.h"
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/string_utils.h"

/**
 * @brief Grammar for parsing timed dates.
 * @details Shall not be modified apart from its creation.
 */
fixed_n_delimiter_parser_grammar_t *date_and_time_grammar = NULL;

/** @brief Timed date with `union`, to easily extract fields from a timed date integer. */
typedef union {
    date_and_time_t date_and_time;

    struct {
        date_t    date;
        daytime_t time;
    } fields;
} date_and_time_union_helper_t;

void date_and_time_from_values(date_and_time_t *output, date_t date, daytime_t time) {
    date_and_time_union_helper_t date_and_time_union;
    date_and_time_union.fields.date = date;
    date_and_time_union.fields.time = time;
    *output                         = date_and_time_union.date_and_time;
}

/**
 * @brief Auxiliary method for ::date_and_time_from_string. Parses dates.
 */
int __date_and_time_from_string_parse_date(void *date_and_time_data, char *token, size_t ntoken) {
    (void) ntoken;
    date_and_time_union_helper_t *date_and_time =
        (date_and_time_union_helper_t *) date_and_time_data;

    return date_from_string(&date_and_time->fields.date, token);
}

/**
 * @brief Auxiliary method for ::date_and_time_from_string. Parses times in the day.
 */
int __date_and_time_from_string_parse_daytime(void  *date_and_time_data,
                                              char  *token,
                                              size_t ntoken) {
    (void) ntoken;
    date_and_time_union_helper_t *date_and_time =
        (date_and_time_union_helper_t *) date_and_time_data;

    return daytime_from_string(&date_and_time->fields.time, token);
}

/**
 * @brief Method used with `atexit` to free the grammar created by ::date_and_time_from_string.
 */
void __date_and_time_from_string_free_grammar(void) {
    free(date_and_time_grammar);
}

int date_and_time_from_string(date_and_time_t *output, char *input) {
    /* Create grammar if needed */
    if (!date_and_time_grammar) {
        fixed_n_delimiter_parser_iter_callback_t callbacks[2] = {
            __date_and_time_from_string_parse_date,
            __date_and_time_from_string_parse_daytime};

        /* NOTE - Make thread-safe if multithreading gets implemented */
        date_and_time_grammar = fixed_n_delimiter_parser_grammar_new(' ', 2, callbacks);
        if (!date_and_time_grammar) {
            return 1;
        }
        atexit(__date_and_time_from_string_free_grammar);
    }

    /* Actually parse date */
    date_and_time_union_helper_t tmp_date;
    int retval = fixed_n_delimiter_parser_parse_string(input, date_and_time_grammar, &tmp_date);
    if (retval) {
        return retval;
    } else {
        *output = tmp_date.date_and_time;
        return 0;
    }
}

int date_and_time_from_string_const(date_and_time_t *output, const char *input) {
    char *buffer = strdup(input);
    if (!buffer)
        return 1;

    int retval = date_and_time_from_string(output, buffer);

    free(buffer);
    return retval;
}

void date_and_time_sprintf(char *output, date_and_time_t date_and_time) {
    date_and_time_union_helper_t date_and_time_union = {.date_and_time = date_and_time};

    char date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(date_str, date_and_time_union.fields.date);

    char time_str[DAYTIME_SPRINTF_MIN_BUFFER_SIZE];
    daytime_sprintf(time_str, date_and_time_union.fields.time);

    sprintf(output, "%s %s", date_str, time_str);
}

int64_t date_and_time_diff(date_and_time_t a, date_and_time_t b) {
    date_and_time_union_helper_t a_union = {.date_and_time = a};
    date_and_time_union_helper_t b_union = {.date_and_time = b};

    return date_diff(a_union.fields.date, b_union.fields.date) * (24 * 60 * 60) +
           daytime_diff(a_union.fields.time, b_union.fields.time);
}

date_t date_and_time_get_date(date_and_time_t date_and_time) {
    date_and_time_union_helper_t date_and_time_union = {.date_and_time = date_and_time};
    return date_and_time_union.fields.date;
}

void date_and_time_set_date(date_and_time_t *date_and_time, date_t date) {
    date_and_time_union_helper_t date_and_time_union = {.date_and_time = *date_and_time};
    date_and_time_union.fields.date                  = date;
    *date_and_time                                   = date_and_time_union.date_and_time;
}

daytime_t date_and_time_get_time(date_and_time_t date_and_time) {
    date_and_time_union_helper_t date_and_time_union = {.date_and_time = date_and_time};
    return date_and_time_union.fields.time;
}

void date_and_time_set_time(date_and_time_t *date_and_time, daytime_t time) {
    date_and_time_union_helper_t date_and_time_union = {.date_and_time = *date_and_time};
    date_and_time_union.fields.time                  = time;
    *date_and_time                                   = date_and_time_union.date_and_time;
}
