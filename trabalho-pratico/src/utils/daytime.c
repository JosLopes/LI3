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
 * @file  daytime.c
 * @brief Implementation of methods in include/utils/daytime.h
 *
 * ### Examples
 * See [the header file's documentation](@ref daytime_examples).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/daytime.h"
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/int_utils.h"
#include "utils/string_utils.h"

/**
 * @brief Grammar for parsing times.
 * @details Shall not be modified apart from its creation.
 *
 * TODO - ask professors if we can use global variables
 */
fixed_n_delimiter_parser_grammar_t *daytime_grammar = NULL;

/** @brief Time with `union`, to easily extract fields from a time integer. */
typedef union {
    daytime_t daytime;

    struct {
        uint8_t hours, minutes, seconds;
    } fields;
} daytime_union_helper_t;

/** @brief The maximum value (inclusive) that hours in a time may take. */
#define DAYTIME_HOURS_MAX   23
/** @brief The maximum value (inclusive) that minutes in a time may take. */
#define DAYTIME_MINUTES_MAX 59
/** @brief The maximum value (inclusive) that seconds in a time may take. */
#define DAYTIME_SECONDS_MAX 59

int daytime_from_values(daytime_t *output, uint8_t hours, uint8_t minutes, uint8_t seconds) {
    daytime_union_helper_t daytime_union;

    if (hours > DAYTIME_HOURS_MAX || minutes > DAYTIME_MINUTES_MAX ||
        seconds > DAYTIME_SECONDS_MAX) {
        return 1;
    }

    daytime_union.fields.hours   = hours;
    daytime_union.fields.minutes = minutes;
    daytime_union.fields.seconds = seconds;

    *output = daytime_union.daytime;
    return 0;
}

/**
 * @brief Auxiliary method for ::daytime_from_string. Parses any of the integers in a time.
 */
int __daytime_from_string_parse_field(void *daytime_data, char *token, size_t ntoken) {
    daytime_union_helper_t *daytime = (daytime_union_helper_t *) daytime_data;

    uint64_t maxs[3]    = {DAYTIME_HOURS_MAX, DAYTIME_MINUTES_MAX, DAYTIME_SECONDS_MAX};
    uint8_t *outputs[3] = {&daytime->fields.hours,
                           &daytime->fields.minutes,
                           &daytime->fields.seconds};

    size_t token_length = strlen(token);
    if (token_length != 2)
        return 1;

    uint64_t parsed           = 0;
    int      int_parse_result = int_utils_parse_positive(&parsed, token);
    if (int_parse_result || parsed > maxs[ntoken]) {
        return 1;
    }

    *(outputs[ntoken]) = parsed;
    return 0;
}

/**
 * @brief Method used with `atexit` to free the grammar created by ::daytime_from_string.
 */
void __daytime_from_string_free_grammar(void) {
    free(daytime_grammar);
}

int daytime_from_string(daytime_t *output, char *input) {
    /* Create grammar if needed */
    if (!daytime_grammar) {
        fixed_n_delimiter_parser_iter_callback_t callbacks[3] = {__daytime_from_string_parse_field,
                                                                 __daytime_from_string_parse_field,
                                                                 __daytime_from_string_parse_field};

        /* NOTE - Make thread-safe if multithreading gets implemented */
        daytime_grammar = fixed_n_delimiter_parser_grammar_new(':', 3, callbacks);
        if (!daytime_grammar) {
            return 1;
        }
        atexit(__daytime_from_string_free_grammar);
    }

    /* Actually parse time */
    daytime_union_helper_t tmp_daytime;
    int retval = fixed_n_delimiter_parser_parse_string(input, daytime_grammar, &tmp_daytime);
    if (retval) {
        return retval;
    } else {
        *output = tmp_daytime.daytime;
        return 0;
    }
}

int daytime_from_string_const(daytime_t *output, const char *input) {
    char *buffer = string_duplicate(input);
    if (!buffer)
        return 1;

    int retval = daytime_from_string(output, buffer);

    free(buffer);
    return retval;
}

void daytime_sprintf(char *output, daytime_t daytime) {
    daytime_union_helper_t daytime_union = {.daytime = daytime};

    sprintf(output,
            "%02d:%02d:%02d",
            daytime_union.fields.hours,
            daytime_union.fields.minutes,
            daytime_union.fields.seconds);
}

uint32_t daytime_diff(daytime_t a, daytime_t b) {
    daytime_union_helper_t a_union = {.daytime = a};
    daytime_union_helper_t b_union = {.daytime = b};

    uint32_t a_seconds =
        a_union.fields.hours * 3600 + a_union.fields.minutes * 60 + a_union.fields.seconds;
    uint32_t b_seconds =
        b_union.fields.hours * 3600 + b_union.fields.minutes * 60 + b_union.fields.seconds;

    return a_seconds - b_seconds;
}

/**
 * @brief Helper macro for defining getters.
 * @param property Property to get in ::daytime_union_helper_t.fields.
 */
#define DAYTIME_GETTER_FUNCTION_BODY(property)                                                     \
    daytime_union_helper_t daytime_union = {.daytime = time};                                      \
    return daytime_union.fields.property;

/**
 * @brief Helper macro for defining setters.
 * @param property Property to set in ::daytime_union_helper_t.fields. Name must match the name of
 *                 the argument in the setter method.
 * @param upper_bound Maximum value (inclusive) that @p property can take.
 */
#define DAYTIME_SETTER_FUNCTION_BODY(property, upper_bound)                                        \
    if (property > upper_bound) {                                                                  \
        return 1;                                                                                  \
    }                                                                                              \
                                                                                                   \
    daytime_union_helper_t daytime_union = {.daytime = *time};                                     \
    daytime_union.fields.property        = property;                                               \
    *time                                = daytime_union.daytime;                                  \
    return 0;

uint8_t daytime_get_hours(daytime_t time) {
    DAYTIME_GETTER_FUNCTION_BODY(hours);
}

int daytime_set_hours(daytime_t *time, uint8_t hours) {
    DAYTIME_SETTER_FUNCTION_BODY(hours, DAYTIME_HOURS_MAX);
}

uint8_t daytime_get_minutes(daytime_t time) {
    DAYTIME_GETTER_FUNCTION_BODY(minutes);
}

int daytime_set_minutes(daytime_t *time, uint8_t minutes) {
    DAYTIME_SETTER_FUNCTION_BODY(minutes, DAYTIME_MINUTES_MAX);
}

uint8_t daytime_get_seconds(daytime_t time) {
    DAYTIME_GETTER_FUNCTION_BODY(seconds);
}

int daytime_set_seconds(daytime_t *time, uint8_t seconds) {
    DAYTIME_SETTER_FUNCTION_BODY(seconds, DAYTIME_SECONDS_MAX);
}
