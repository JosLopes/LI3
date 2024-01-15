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
 * @file  flights_loader.c
 * @brief Implementation of methods in include/dataset/flights_loader.h
 *
 * @details Many internal methods in this module are lacking parameter documentation, as they all
 *          follow the same convention: all are ::fixed_n_delimiter_parser_iter_callback_t. The
 *          `loader_data` is a pointer to a ::flights_loader_t.
 */

#include <string.h>

#include "dataset/dataset_parser.h"
#include "dataset/flights_loader.h"
#include "utils/int_utils.h"

/** @cond FALSE */
#ifndef __GNUC__
    #define __builtin_unreachable() return 0;
#endif
/** @endcond */

/**
 * @struct flights_loader_t
 * @brief  Temporary data needed to load a set of flights.
 *
 * @var flights_loader_t::output
 *     @brief Where to output dataset errors to.
 * @var flights_loader_t::database
 *     @brief Database to add new flights to.
 * @var flights_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var flights_loader_t::current_flight
 *     @brief Flight being currently parsed, whose fields are still being filled in.
 * @var flights_loader_t::first_line
 *     @brief   Whether the line being parsed is the first line in the file.
 *     @details Used to print an error on the CSV's table header.
 */
typedef struct {
    dataset_error_output_t *const output;
    database_t *const             database;

    const char     *error_line;
    flight_t *const current_flight;
    int             first_line;
} flights_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 *
 * @param loader_data A pointer to a ::flights_loader_t.
 * @param line        Line that is going to be parsed.
 *
 * @retval 0 Always successful.
 */
int __flights_loader_before_parse_line(void *loader_data, char *line) {
    ((flights_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a flight's identifier. */
int __flight_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *const loader = loader_data;

    flight_id_t id;
    const int   retval = flight_id_from_string(&id, token);
    if (retval == 0) {
        flight_set_id(loader->current_flight, id);
    } else if (retval == 2 && !(loader->first_line && strcmp(token, "id") == 0)) {
        fprintf(stderr,
                "Flight ID \"%s\" is not numerical. This isn't supported by our program!\n",
                token);
    }
    return retval;
}

/** @brief Parses a flight's airline and plane model (simple strings). */
int __flight_loader_parse_string(void *loader_data, char *token, size_t ntoken) {
    switch (ntoken) {
        case 1:
            return flight_set_airline(NULL,
                                      ((flights_loader_t *) loader_data)->current_flight,
                                      token);
        case 2:
            return flight_set_plane_model(NULL,
                                          ((flights_loader_t *) loader_data)->current_flight,
                                          token);
        default:
            __builtin_unreachable();
    }
}

/** @brief Parses a flight's total number of seats. */
int __flight_loader_parse_total_seats(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    uint64_t  parsed_total_seats;
    const int retval = int_utils_parse_positive(&parsed_total_seats, token);
    if (retval)
        return retval;

    return flight_set_total_seats(((flights_loader_t *) loader_data)->current_flight,
                                  parsed_total_seats);
}

/** @brief Parses a flight's origin or destination airport. */
int __flight_loader_parse_airport(void *loader_data, char *token, size_t ntoken) {
    flights_loader_t *const loader = loader_data;

    airport_code_t airport_code;
    const int      retval = airport_code_from_string(&airport_code, token);
    if (retval)
        return retval;

    switch (ntoken) {
        case 4:
            flight_set_origin(loader->current_flight, airport_code);
            break;
        case 5:
            flight_set_destination(loader->current_flight, airport_code);
            break;
        default:
            __builtin_unreachable();
    }
    return 0;
}

/** @brief Parses a flight's scheduled departure and arrival date. */
int __flight_loader_parser_schedule_dates(void *loader_data, char *token, size_t ntoken) {
    flights_loader_t *const loader = loader_data;

    date_and_time_t date_and_time;
    const int       retval = date_and_time_from_string(&date_and_time, token);
    if (retval)
        return retval;

    switch (ntoken) {
        case 6:
            return flight_set_schedule_departure_date(loader->current_flight, date_and_time);
        case 7:
            return flight_set_schedule_arrival_date(loader->current_flight, date_and_time);
        default:
            __builtin_unreachable();
    }
}

/** @brief Parses a flight's real departure date. */
int __flight_loader_parse_real_departure_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    date_and_time_t date_and_time;
    const int       retval = date_and_time_from_string(&date_and_time, token);
    if (retval)
        return retval;

    flight_set_real_departure_date(((flights_loader_t *) loader_data)->current_flight,
                                   date_and_time);
    return 0;
}

/** @brief Parses a flight's real arrival date. */
int __flight_loader_parse_real_arrival_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    date_and_time_t date_and_time;
    const int       retval = date_and_time_from_string(&date_and_time, token);
    if (retval)
        return retval;

    /* This check is performed in the parser, as flights don't store this field. */
    const date_and_time_t departure_date =
        flight_get_real_departure_date(((flights_loader_t *) loader_data)->current_flight);
    return date_and_time_diff(date_and_time, departure_date) < 0;
}

/** @brief Parses a flight's pilot and copilot name. */
int __flight_loader_parse_pilot_copilot(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    (void) loader_data;
    return (*token == 0); /* Fail on empty pilot / copilot name */
}

/** @brief Parses a flight's notes. */
int __flight_loader_parse_notes(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    (void) loader_data;
    (void) token;
    return 0;
}

/**
 * @brief Places a parsed flight in the database and handles parsing errors.
 *
 * @param loader_data A pointer to a ::flights_loader_t.
 * @param retval      Value returned by the last token callback (`0` for success, another value for
 *                    a parsing error).
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __flights_loader_after_parse_line(void *loader_data, int retval) {
    flights_loader_t *const loader = loader_data;
    loader->first_line             = 0;

    if (retval) {
        dataset_error_output_report_flight_error(loader->output, loader->error_line);
        retval = 0;
    } else {
        retval = database_add_flight(loader->database, loader->current_flight);
    }

    flight_reset_schedule_dates(loader->current_flight);
    return retval;
}

int flights_loader_load(FILE *stream, database_t *database, dataset_error_output_t *output) {
    flights_loader_t data = {.output         = output,
                             .database       = database,
                             .current_flight = flight_create(NULL),
                             .first_line     = 1};
    if (!data.current_flight)
        return 1;
    flight_set_number_of_passengers(data.current_flight, 0);

    const fixed_n_delimiter_parser_iter_callback_t token_callbacks[13] = {
        __flight_loader_parse_id,
        __flight_loader_parse_string,
        __flight_loader_parse_string,
        __flight_loader_parse_total_seats,
        __flight_loader_parse_airport,
        __flight_loader_parse_airport,
        __flight_loader_parser_schedule_dates,
        __flight_loader_parser_schedule_dates,
        __flight_loader_parse_real_departure_date,
        __flight_loader_parse_real_arrival_date,
        __flight_loader_parse_pilot_copilot,
        __flight_loader_parse_pilot_copilot,
        __flight_loader_parse_notes,
    };

    fixed_n_delimiter_parser_grammar_t *const line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 13, token_callbacks);
    if (!line_grammar) {
        flight_free(data.current_flight);
        return 1;
    }

    dataset_parser_grammar_t *const grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __flights_loader_before_parse_line,
                                   __flights_loader_after_parse_line);
    if (!grammar) {
        flight_free(data.current_flight);
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        return 1;
    }

    const int retval = dataset_parser_parse(stream, grammar, &data);

    dataset_parser_grammar_free(grammar);
    fixed_n_delimiter_parser_grammar_free(line_grammar);
    flight_free(data.current_flight);

    return retval != 0;
}
