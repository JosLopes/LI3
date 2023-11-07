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
 */

#include <ctype.h>
#include <stdio.h> /* For testing purposes */
#include <string.h>

#include "dataset/flights_loader.h"
#include "types/flight.h"
#include "utils/dataset_parser.h"
#include "utils/int_utils.h"

/** @brief Table header for `flights_errors.csv` */
#define FLIGHTS_LOADER_HEADER                                                                      \
    "id;airline;plane_model;total_seats;origin;destination;schedule_departure_date;"               \
    "schedule_arrival_date;real_departure_date;real_arrival_date;pilot;copilot;notes"

/**
 * @struct flights_loader_t
 * @brief  Temporary data needed to load a set of flights.
 *
 * @var flights_loader_t::dataset
 *     @brief Dataset loader, so that errors can be reported.
 * @var flights_loader_t::flights
 *     @brief Flight manager to add new flights to.
 * @var flights_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var flights_loader_t::current_flight
 *     @brief Flight being currently parsed, whose fields are still being filled in.
 * @var flights_loader_t::airline_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the flight's airline name
 *            ends.
 * @var flights_loader_t::plane_model_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the flight's plane model name
 *            ends.
 */
typedef struct {
    dataset_loader_t *dataset;
    flight_manager_t *flights;

    char *error_line;

    flight_t *current_flight;
    char     *airline_terminator, *plane_model_terminator;
} flights_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __flights_loader_before_parse_line(void *loader_data, char *line) {
    ((flights_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a flight's identifier */
int __flight_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    size_t parsed_id;
    int    retcode = int_utils_parse_positive(&parsed_id, token);
    if (!retcode) {
        flight_set_id(loader->current_flight, parsed_id);
        return 0;
    } else {
        if (*token)
            fprintf(stderr,
                    "Non-numerical flight ID detected, \"%s\". Our program's architecture doesn't "
                    "allow for this.\n",
                    token);
        return 1;
    }
}

/** @brief Parses a flight's airline */
int __flight_loader_parse_airline(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        flight_set_airline(loader->current_flight, token);
        loader->airline_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a flight's plane model */
int __flight_loader_parse_plane_model(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        flight_set_plane_model(loader->current_flight, token);
        loader->plane_model_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a flight's number of total seats */
int __flight_loader_parse_total_seats(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    uint64_t parsed_total_seats;
    int      retcode = int_utils_parse_positive(&parsed_total_seats, token);
    if (!retcode) {
        flight_set_total_seats(loader->current_flight, parsed_total_seats);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a flight's origin airport */
int __flight_loader_parse_origin(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    airport_code_t airport_code;
    int            retcode = airport_code_from_string(&airport_code, token);
    if (retcode) {
        return retcode;
    } else {
        flight_set_origin(loader->current_flight, airport_code);
        return 0;
    }
}

/** @brief Parses a flight's destination airport */
int __flight_loader_parse_destination(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    airport_code_t airport_code;
    int            retcode = airport_code_from_string(&airport_code, token);
    if (retcode) {
        return retcode;
    } else {
        flight_set_destination(loader->current_flight, airport_code);
        return 0;
    }
}

/** @brief Parses a flight's scheduled departure date */
int __flight_loader_parse_schedule_departure_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    date_and_time_t date_and_time;
    int             retcode = date_and_time_from_string(&date_and_time, token);
    if (retcode) {
        return retcode;
    } else {
        flight_set_schedule_departure_date(loader->current_flight, date_and_time);
        return 0;
    }
}

/** @brief Parses a flight's scheduled arrival date */
int __flight_loader_parse_schedule_arrival_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    date_and_time_t date_and_time;
    int             retcode = date_and_time_from_string(&date_and_time, token);
    if (retcode) {
        return retcode;
    } else {
        if (date_and_time_diff(date_and_time,
                               flight_get_schedule_departure_date(loader->current_flight)) < 0) {
            return 1; /* Arrival before departure */
        }

        flight_set_schedule_arrival_date(loader->current_flight, date_and_time);
        return 0;
    }
}

/** @brief Parses a flight's real departure date */
int __flight_loader_parse_real_departure_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    date_and_time_t date_and_time;
    int             retcode = date_and_time_from_string(&date_and_time, token);
    if (retcode) {
        return retcode;
    } else {
        flight_set_real_departure_date(loader->current_flight, date_and_time);
        return 0;
    }
}

/** @brief Parses a flight's real arrival date */
int __flight_loader_parse_real_arrival_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    date_and_time_t date_and_time;
    int             retcode = date_and_time_from_string(&date_and_time, token);

    if (!retcode) {
        if (date_and_time_diff(date_and_time,
                               flight_get_real_departure_date(loader->current_flight)) < 0) {
            return 1; /* Arrival before departure */
        }
        return 0;
    } else {
        return retcode;
    }
}

/** @brief Parses a flight's pilot name */
int __flight_loader_parse_pilot(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    (void) loader_data;
    return (*token == 0); /* Fail on empty pilot name */
}

/** @brief Parses a flight's copilot name */
int __flight_loader_parse_copilot(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    (void) loader_data;
    return (*token == 0); /* Fail on empty copilot name */
}

/** @brief Parses a flight's notes */
int __flight_loader_parse_notes(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    (void) loader_data;
    (void) token;
    return 0;
}

/** @brief Places a parsed flight in the database and handles errors */
int __flights_loader_after_parse_line(void *loader_data, int retval) {
    flights_loader_t *loader = (flights_loader_t *) loader_data;

    if (retval) {
        dataset_loader_report_flights_error(loader->dataset, loader->error_line);
    } else {
        /* Restore token terminations for strings that will be stored in a flight. */
        *loader->airline_terminator     = '\0';
        *loader->plane_model_terminator = '\0';

        flight_set_number_of_passengers(loader->current_flight, 0);
        flight_manager_add_flight(loader->flights, loader->current_flight);
    }
    return 0;
}

void flights_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_flights_error(dataset_loader, FLIGHTS_LOADER_HEADER);
    flights_loader_t data = {.dataset = dataset_loader,
                             .flights =
                                 database_get_flights(dataset_loader_get_database(dataset_loader)),
                             .current_flight = flight_create()};

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[13] = {
        __flight_loader_parse_id,
        __flight_loader_parse_airline,
        __flight_loader_parse_plane_model,
        __flight_loader_parse_total_seats,
        __flight_loader_parse_origin,
        __flight_loader_parse_destination,
        __flight_loader_parse_schedule_departure_date,
        __flight_loader_parse_schedule_arrival_date,
        __flight_loader_parse_real_departure_date,
        __flight_loader_parse_real_arrival_date,
        __flight_loader_parse_pilot,
        __flight_loader_parse_copilot,
        __flight_loader_parse_notes,
    };

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 13, token_callbacks);

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __flights_loader_before_parse_line,
                                   __flights_loader_after_parse_line);

    dataset_parser_parse(stream, grammar, &data);
    dataset_parser_grammar_free(grammar);
    flight_free(data.current_flight);
}
