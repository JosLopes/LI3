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

/**
 * @union daytime_union_helper_t
 * @brief Time with `union`, to easily extract fields from a time integer.
 *
 * @var daytime_union_helper_t::daytime
 *     @brief Compact time format, exposed to the outside of this module.
 * @var daytime_union_helper_t::fields
 *     @brief Individual fields within ::daytime_union_helper_t::daytime.
 * @var daytime_union_helper_t::hours
 *     @brief Hour in @p ::daytime_union_helper_t::daytime. Must be between `0` and
 *            ::DAYTIME_HOURS_MAX.
 * @var daytime_union_helper_t::minutes
 *     @brief Minute in @p ::daytime_union_helper_t::daytime. Must be between `0` and
 *            ::DAYTIME_MINUTES_MAX.
 * @var daytime_union_helper_t::seconds
 *     @brief Second in @p ::daytime_union_helper_t::daytime. Must be between `0` and
 *            ::DAYTIME_SECONDS_MAX.
 */
typedef union {
    const daytime_t daytime;

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
 *
 * @param daytime_data A pointer to a ::daytime_union_helper_t, whose fields are filled in as the
 *                     time is parsed.
 * @param token        Number between colons to be parsed.
 * @param ntoken       Tokens already parsed (number of the current token, `0`-indexed).
 *
 * @retval 0 Success.
 * @retval 1 Integer parsing failure.
 */
int __daytime_from_string_parse_field(void *daytime_data, char *token, size_t ntoken) {
    daytime_union_helper_t *const daytime = daytime_data;

    const uint64_t maxs[3]    = {DAYTIME_HOURS_MAX, DAYTIME_MINUTES_MAX, DAYTIME_SECONDS_MAX};
    uint8_t       *outputs[3] = {&daytime->fields.hours,
                                 &daytime->fields.minutes,
                                 &daytime->fields.seconds};

    const size_t token_length = strlen(token);
    if (token_length != 2)
        return 1;

    uint64_t  parsed           = 0;
    const int int_parse_result = int_utils_parse_positive(&parsed, token);
    if (int_parse_result || parsed > maxs[ntoken]) {
        return 1;
    }

    *(outputs[ntoken]) = parsed;
    return 0;
}

/**
 * @brief   Grammar for parsing times.
 * @details Shall not be modified apart from its creation. It's not constant because it requires
 *          run-time initialization. This global variable is justified for the following reasons:
 *
 *          -# It's not modified (no mutable global state);
 *          -# It's module-local (no breaking of encapsulation);
 *          -# Helps performance, as a new grammar doesn't need to be generated for every time to
 *             be parsed.
 */
fixed_n_delimiter_parser_grammar_t *__daytime_grammar = NULL;

/** @brief Automatically initializes ::__daytime_grammar when the program starts. */
void __attribute__((constructor)) __daytime_grammar_create(void) {
    const fixed_n_delimiter_parser_iter_callback_t callbacks[3] = {
        __daytime_from_string_parse_field,
        __daytime_from_string_parse_field,
        __daytime_from_string_parse_field};

    __daytime_grammar = fixed_n_delimiter_parser_grammar_new(':', 3, callbacks);
}

/** @brief Automatically frees ::__daytime_grammar when the program terminates */
void __attribute__((destructor)) __daytime_grammar_free(void) {
    fixed_n_delimiter_parser_grammar_free(__daytime_grammar);
}

int daytime_from_string(daytime_t *output, char *input) {
    daytime_union_helper_t tmp_daytime;
    const int              retval =
        fixed_n_delimiter_parser_parse_string(input, __daytime_grammar, &tmp_daytime);
    if (retval) {
        return retval;
    } else {
        *output = tmp_daytime.daytime;
        return 0;
    }
}

int daytime_from_string_const(daytime_t *output, const char *input) {
    char *const buffer = strdup(input);
    if (!buffer)
        return 1;

    const int retval = daytime_from_string(output, buffer);

    free(buffer);
    return retval;
}

void daytime_sprintf(char *output, daytime_t daytime) {
    const daytime_union_helper_t daytime_union = {.daytime = daytime};

    sprintf(output,
            "%02d:%02d:%02d",
            daytime_union.fields.hours,
            daytime_union.fields.minutes,
            daytime_union.fields.seconds);
}

int32_t daytime_diff(daytime_t a, daytime_t b) {
    const daytime_union_helper_t a_union = {.daytime = a};
    const daytime_union_helper_t b_union = {.daytime = b};

    const uint32_t a_seconds = (uint32_t) a_union.fields.hours * 3600 +
                               (uint32_t) a_union.fields.minutes * 60 +
                               (uint32_t) a_union.fields.seconds;
    const uint32_t b_seconds = (uint32_t) b_union.fields.hours * 3600 +
                               (uint32_t) b_union.fields.minutes * 60 +
                               (uint32_t) b_union.fields.seconds;

    return a_seconds - b_seconds;
}

/**
 * @brief Helper macro for defining getters.
 * @param property Property to get in ::daytime_union_helper_t::fields.
 */
#define DAYTIME_GETTER_FUNCTION_BODY(property)                                                     \
    const daytime_union_helper_t daytime_union = {.daytime = time};                                \
    return daytime_union.fields.property;

/**
 * @brief Helper macro for defining setters.
 * @param property    Property to set in ::daytime_union_helper_t::fields. Name must match the name
 *                    of the argument in the setter method.
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
