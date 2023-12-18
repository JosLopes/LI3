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
 * @file  passengers_loader.c
 * @brief Implementation of methods in include/dataset/passengers_loader.h
 */

#include <ctype.h>
#include <glib.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>

#include "dataset/dataset_parser.h"
#include "dataset/passengers_loader.h"
#include "utils/int_utils.h"
#include "utils/stream_utils.h"
#include "utils/string_pool.h"

/** @brief Table header for `passengers_errors.csv` */
#define PASSENGERS_LOADER_HEADER "flight_id;user_id"

/**
 * @struct passenger_relation_t
 * @brief A passenger in a flight.
 *
 * @var passenger_relation_t::user_id
 *     @brief User in the flight identifiable by ::passenger_relation_t::flight_id.
 * @var passenger_relation_t::flight_id
 *     @brief Flight the user identifiable by ::passenger_relation_t::user_id is in.
 */
typedef struct {
    const char *user_id;
    flight_id_t flight_id;
} passenger_relation_t;

/** @brief Block capacity of ::passengers_loader_t::commit_buffer_id_pool */
#define PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY 8192

/**
 * @struct passengers_loader_t
 * @brief  Temporary data needed to load a set of passengers.
 *
 * @var passengers_loader_t::output
 *     @brief Where to output dataset errors to.
 * @var passengers_loader_t::users
 *     @brief User manager to add new passenger relationships to.
 * @var passengers_loader_t::flights
 *     @brief Flight manager to check for flight existence.
 * @var passengers_loader_t::commit_buffer
 *     @brief   All passengers in the flight (user IDs) being currently parsed.
 *     @details These will only be committed to the database when we make sure there are no more
 *              passengers than seats.
 * @var passengers_loader_t::commit_buffer_flight
 *     @brief Flight ::passengers_loader_t::commit_buffer refers to.
 * @var passengers_loader_t::commit_buffer_id_pool
 *     @brief Pool where IDs in ::passengers_loader_t::commit_buffer are stored.
 * @var passengers_loader_t::current_relation
 *     @brief Current user-flight relation being parsed.
 * @var passengers_loader_t::invalid_flight_ids
 *     @brief List of invalid flight IDs to be printed to the errors file.
 * @var passengers_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 */
typedef struct {
    dataset_error_output_t *output;
    user_manager_t         *users;
    flight_manager_t       *flights;

    GPtrArray     *commit_buffer;
    flight_id_t    commit_buffer_flight;
    string_pool_t *commit_buffer_id_pool;

    passenger_relation_t current_relation;

    GArray     *invalid_flight_ids;
    const char *error_line;
} passengers_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __passengers_loader_before_parse_line(void *loader_data, char *line) {
    ((passengers_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a user flight in a user-flight passenger relation. */
int __passengers_loader_parse_flight_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    flight_id_t id;
    int         retval = flight_id_from_string(&id, token);
    if (retval == 0) {
        loader->current_relation.flight_id = id;

        /* Fail if the flight isn't found (invalid flights won't be found too). */
        return flight_manager_get_by_id(loader->flights, id) == NULL;
        return 0;
    } else if (retval == 2) {
        fprintf(stderr,
                "Flight ID \"%s\" is not numerical. This isn't supported by our program!\n",
                token);
    }
    return 1;
}

/** @brief Parses a user id in a user-flight passenger relation. */
int __passengers_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    loader->current_relation.user_id = token;

    /* Fail if the user isn't found (invalid user won't be found too). */
    return user_manager_get_by_id(loader->users, token) == NULL;
}

/**
 * @brief Adds all passengers in a commit buffer to the database.
 * @param loader Current state of the loader of the passengers file.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __passengers_loader_commit_flight_list(passengers_loader_t *loader) {
    flight_t *flight = flight_manager_get_by_id(loader->flights, loader->commit_buffer_flight);
    flight_set_number_of_passengers(flight, loader->commit_buffer->len);

    if ((guint) flight_get_total_seats(flight) < loader->commit_buffer->len) {
        flight_manager_invalidate_by_id(loader->flights, loader->commit_buffer_flight);
        g_array_append_val(loader->invalid_flight_ids, loader->commit_buffer_flight);

        /* Print passenger entries as invalid */
        char print_buffer[LINE_MAX];
        for (size_t i = 0; i < loader->commit_buffer->len; ++i) {
            char *user_id = g_ptr_array_index(loader->commit_buffer, i);
            char  flight_id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
            flight_id_sprintf(flight_id_str, loader->commit_buffer_flight);

            snprintf(print_buffer, LINE_MAX, "%s;%s", flight_id_str, user_id);
            dataset_error_output_report_passenger_error(loader->output, print_buffer);
        }
    } else {
        flight_id_t flight_id = flight_get_id(flight);
        for (size_t i = 0; i < loader->commit_buffer->len; ++i) {
            const char *user_id = g_ptr_array_index(loader->commit_buffer, i);
            /* Ignore allocation errors */
            if (user_manager_add_user_flight_association(loader->users, user_id, flight_id))
                return 1;
        }
    }

    return 0;
}

/** @brief Places a parsed passenger in the database and handles errors */
int __passengers_loader_after_parse_line(void *loader_data, int retval) {
    passengers_loader_t *loader = (passengers_loader_t *) loader_data;

    if (retval) {
        dataset_error_output_report_passenger_error(loader->output, loader->error_line);
        return 0;
    }

    if (loader->commit_buffer->len != 0 &&
        loader->current_relation.flight_id != loader->commit_buffer_flight) {
        /* New flight */

        if (__passengers_loader_commit_flight_list(loader))
            return 1; /* Allocation error */

        string_pool_empty(loader->commit_buffer_id_pool);
        g_ptr_array_set_size(loader->commit_buffer, 0);
    }

    /* Still the same flight */
    char *pool_alloc_id =
        string_pool_put(loader->commit_buffer_id_pool, loader->current_relation.user_id);
    g_ptr_array_add(loader->commit_buffer, pool_alloc_id);
    loader->commit_buffer_flight = loader->current_relation.flight_id;

    return 0;
}

/**
 * @struct passengers_loader_erroneous_flight_callback_data_t
 * @brief  Type of `user_data` parameter in ::__passengers_loader_report_erroneous_flight.
 *
 * @var passengers_loader_erroneous_flight_callback_data_t::output
 *     @brief Where to output dataset errors to.
 * @var passengers_loader_erroneous_flight_callback_data_t::flight_id
 *     @brief Identifier of the flight currently being looked up in the dataset file.
 */
typedef struct {
    dataset_error_output_t *output;
    const char             *flight_id;
} passengers_loader_erroneous_flight_callback_data_t;

/**
 * @brief Checks if a given line of the dataset contains the desired flight, reporting it as an
 *        error if that's the case.
 *
 * @param user_data A pointer to a ::passengers_loader_erroneous_flight_callback_data_t (this
 *                  includes the ID of the desired erroneous flight).
 * @param line      Current line in the `flights.csv` dataset file.
 *
 * @retval 0 Continue flight lookup.
 * @retval 1 Flight found. Stop lookup.
 */
int __passengers_loader_check_line_for_erroneous_flight(void *user_data, char *line) {
    passengers_loader_erroneous_flight_callback_data_t *data =
        (passengers_loader_erroneous_flight_callback_data_t *) user_data;

    if (strncmp(data->flight_id, line, 10) == 0) { /* Flights always have 10-digit IDs */
        dataset_error_output_report_flight_error(data->output, line);
        return 1; /* Stop looking */
    } else {
        return 0;
    }
}

/**
 * @brief Reports all flights with more passengers than seats.
 *
 * @param loader  Data about parsing results.
 * @param flights File with the flights, to read the correct lines to print to the error file.
 */
void __passengers_loader_report_erroneous_flights(passengers_loader_t *loader, FILE *flights) {
    for (size_t i = 0; i < loader->invalid_flight_ids->len; ++i) {
        flight_id_t id_int = g_array_index(loader->invalid_flight_ids, flight_id_t, i);
        char        flight_id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
        flight_id_sprintf(flight_id_str, id_int);

        passengers_loader_erroneous_flight_callback_data_t user_data = {.output    = loader->output,
                                                                        .flight_id = flight_id_str};

        stream_tokenize(flights,
                        '\n',
                        __passengers_loader_check_line_for_erroneous_flight,
                        &user_data);
    }
}

int passengers_loader_load(FILE                   *passengers_stream,
                           FILE                   *flights_stream,
                           database_t             *database,
                           dataset_error_output_t *output) {

    dataset_error_output_report_passenger_error(output, PASSENGERS_LOADER_HEADER);
    passengers_loader_t data = {.output        = output,
                                .users         = database_get_users(database),
                                .flights       = database_get_flights(database),
                                .commit_buffer = g_ptr_array_new()};

    data.commit_buffer_id_pool = string_pool_create(PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY);
    if (!data.commit_buffer_id_pool) { /* Allocation failure */
        g_ptr_array_unref(data.commit_buffer);
        return 1;
    }

    data.invalid_flight_ids = g_array_new(FALSE, FALSE, sizeof(flight_id_t));

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[2] = {
        __passengers_loader_parse_flight_id,
        __passengers_loader_parse_user_id};

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 2, token_callbacks);
    if (!line_grammar) {
        g_ptr_array_unref(data.commit_buffer);
        string_pool_free(data.commit_buffer_id_pool);
        g_array_unref(data.invalid_flight_ids);
        return 1;
    }

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __passengers_loader_before_parse_line,
                                   __passengers_loader_after_parse_line);
    if (!grammar) {
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        g_ptr_array_unref(data.commit_buffer);
        string_pool_free(data.commit_buffer_id_pool);
        g_array_unref(data.invalid_flight_ids);
        return 1;
    }

    int retval = dataset_parser_parse(passengers_stream, grammar, &data);
    if (data.commit_buffer->len > 0) /* Don't fail on empty files */
        __passengers_loader_commit_flight_list(&data);
    __passengers_loader_report_erroneous_flights(&data, flights_stream);

    g_ptr_array_unref(data.commit_buffer);
    string_pool_free(data.commit_buffer_id_pool);
    g_array_unref(data.invalid_flight_ids);
    fixed_n_delimiter_parser_grammar_free(line_grammar);
    dataset_parser_grammar_free(grammar);

    return retval;
}
