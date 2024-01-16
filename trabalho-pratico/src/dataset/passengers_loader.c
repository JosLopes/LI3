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

#include <glib.h>
#include <limits.h>

#include "dataset/dataset_parser.h"
#include "dataset/passengers_loader.h"
#include "utils/stream_utils.h"
#include "utils/string_pool.h"

/** @brief Block capacity of ::passengers_loader_t::commit_buffer_id_pool. */
#define PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY 8192

/**
 * @struct passengers_loader_t
 * @brief  Temporary data needed to load a set of passengers.
 *
 * @var passengers_loader_t::output
 *     @brief Where to output dataset errors to.
 * @var passengers_loader_t::database
 *     @brief Database to add user-flight relations (passengers) to.
 * @var passengers_loader_t::users
 *     @brief User manager to check for user existence.
 * @var passengers_loader_t::flights
 *     @brief Flight manager to check for flight existence.
 * @var passengers_loader_t::commit_buffer
 *     @brief All passengers (user ID strings) in the flight being currently parsed.
 * @var passengers_loader_t::commit_buffer_flight
 *     @brief Flight that ::passengers_loader_t::commit_buffer refers to.
 * @var passengers_loader_t::commit_buffer_id_pool
 *     @brief Pool where IDs in ::passengers_loader_t::commit_buffer are stored.
 * @var passengers_loader_t::current_user
 *     @brief User ID in the line currently being parsed.
 * @var passengers_loader_t::current_flight
 *     @brief Flight ID in the line currently being parsed.
 * @var passengers_loader_t::invalid_flight_ids
 *     @brief   List of invalid flight IDs to be printed to the errors file.
 *     @details That is done only after fully loading the `passengers.csv` file.
 * @var passengers_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var passengers_loader_t::first_line
 *     @brief   Whether the line being parsed is the first line in the file.
 *     @details Used to print an error on the CSV's table header.
 */
typedef struct {
    dataset_error_output_t *const output;
    database_t *const             database;
    const user_manager_t *const   users;
    const flight_manager_t *const flights;

    GPtrArray *const commit_buffer;
    flight_id_t      commit_buffer_flight;
    string_pool_t   *commit_buffer_id_pool;

    const char *current_user;
    flight_id_t current_flight;

    GArray *const invalid_flight_ids;
    const char   *error_line;
    int           first_line;
} passengers_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 *
 * @param loader_data A pointer to a ::flights_loader_t.
 * @param line        Line that is going to be parsed.
 *
 * @retval 0 Always successful.
 */
int __passengers_loader_before_parse_line(void *loader_data, char *line) {
    ((passengers_loader_t *) loader_data)->error_line = line;
    return 0;
}

/**
 * @brief Parses a flight's identifier in a user-flight relation (passenger).
 *
 * @param loader_data A pointer to a ::passengers_loader_t.
 * @param token       Identifier of the flight as a string.
 * @param ntoken      Number of the current token in the line. Not used.
 *
 * @retval 0 Parsing success.
 * @retval 1 Parsing failure.
 */
int __passengers_loader_parse_flight_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *const loader = loader_data;

    flight_id_t id;
    const int   retval = flight_id_from_string(&id, token);
    if (retval == 0) {
        loader->current_flight = id;
        return flight_manager_get_by_id(loader->flights, id) == NULL;
    } else if (retval == 2 && !(loader->first_line && strcmp(token, "flight_id") == 0)) {
        fprintf(stderr,
                "Flight ID \"%s\" is not numerical. This isn't supported by our program!\n",
                token);
    }
    return retval;
}

/**
 * @brief Parses a user's identifier in a user-flight relation (passenger).
 *
 * @param loader_data A pointer to a ::passengers_loader_t.
 * @param token       Identifier of the user.
 * @param ntoken      Number of the current token in the line. Not used.
 *
 * @retval 0 Parsing success.
 * @retval 1 User doesn't exist.
 */
int __passengers_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    passengers_loader_t *const loader = loader_data;

    /* Fail if the user isn't found (invalid user won't be found too). */
    loader->current_user = token;
    return user_manager_get_by_id(loader->users, token) == NULL;
}

/**
 * @brief Adds all passengers in the commit buffer to the database.
 * @param loader Current state of the loader of the passengers file.
 */
void __passengers_loader_commit_flight_list(passengers_loader_t *loader) {
    if (loader->commit_buffer->len == 0)
        return;

    if (database_add_passengers(loader->database,
                                loader->commit_buffer_flight,
                                loader->commit_buffer->len,
                                (const char *const *) loader->commit_buffer->pdata)) {

        /* Print passengers as invalid (ignore allocation failures) */
        char full_print_buffer[LINE_MAX];
        flight_id_sprintf(full_print_buffer, loader->commit_buffer_flight);
        /* Flight IDs are 10 characters long */
        full_print_buffer[10]    = ';';
        char *const print_buffer = full_print_buffer + 11;

        for (size_t i = 0; i < loader->commit_buffer->len; ++i) {
            strcpy(print_buffer, g_ptr_array_index(loader->commit_buffer, i));
            dataset_error_output_report_passenger_error(loader->output, full_print_buffer);
        }

        g_array_append_val(loader->invalid_flight_ids, loader->commit_buffer_flight);
    }

    string_pool_empty(loader->commit_buffer_id_pool);
    g_ptr_array_set_size(loader->commit_buffer, 0);
}

/**
 * @brief Places a parsed passenger in the database and handles errors.
 *
 * @param loader_data A pointer to a ::passengers_loader_t.
 * @param retval      Value returned by the last token callback (`0` for success, another value for
 *                    a parsing error).
 *
 * @retval 0 Always successful. A possible type of allocation error is ignored.
 */
int __passengers_loader_after_parse_line(void *loader_data, int retval) {
    passengers_loader_t *const loader = loader_data;
    loader->first_line                = 0;

    if (retval) {
        dataset_error_output_report_passenger_error(loader->output, loader->error_line);
        return 0;
    }

    /* Flush passengers if this a new flight */
    if (loader->current_flight != loader->commit_buffer_flight)
        __passengers_loader_commit_flight_list(loader);

    /* Add flight */
    char *const pool_alloc_id =
        string_pool_put(loader->commit_buffer_id_pool, loader->current_user);
    g_ptr_array_add(loader->commit_buffer, pool_alloc_id);
    loader->commit_buffer_flight = loader->current_flight;

    return 0;
}

/**
 * @struct passengers_loader_erroneous_flight_callback_data_t
 * @brief  Type of `user_data` parameter in ::__passengers_loader_check_line_for_erroneous_flight.
 *
 * @var passengers_loader_erroneous_flight_callback_data_t::output
 *     @brief Where to output dataset errors to.
 * @var passengers_loader_erroneous_flight_callback_data_t::flight_id
 *     @brief Identifier of the flight currently being looked up in the dataset file.
 */
typedef struct {
    dataset_error_output_t *const output;
    const char *const             flight_id;
} passengers_loader_erroneous_flight_callback_data_t;

/**
 * @brief   Checks if a given line of the dataset contains the desired flight to be reported.
 * @details It then reports it as an error if that's the case.
 *
 * @param user_data A pointer to a ::passengers_loader_erroneous_flight_callback_data_t.
 * @param line      Current line in the `flights.csv` dataset file.
 *
 * @retval 0 Continue flight lookup.
 * @retval 1 Flight found. Stop lookup.
 */
int __passengers_loader_check_line_for_erroneous_flight(void *user_data, char *line) {
    const passengers_loader_erroneous_flight_callback_data_t *const data = user_data;

    if (strncmp(data->flight_id, line, 10) == 0) { /* Flights always have 10-digit IDs */
        dataset_error_output_report_flight_error(data->output, line);
        return 1; /* Stop looking */
    } else {
        return 0;
    }
}

/**
 * @brief Reports errors for all flights with more passengers than seats.
 *
 * @param loader  Data about parsing results.
 * @param flights File stream of `flights.csv`, to print error lines exactly like they were in the
 *                original dataset file.
 */
void __passengers_loader_report_erroneous_flights(passengers_loader_t *loader, FILE *flights) {
    for (size_t i = 0; i < loader->invalid_flight_ids->len; ++i) {
        const flight_id_t id = g_array_index(loader->invalid_flight_ids, flight_id_t, i);
        char              id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
        flight_id_sprintf(id_str, id);

        passengers_loader_erroneous_flight_callback_data_t user_data = {.output    = loader->output,
                                                                        .flight_id = id_str};

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

    passengers_loader_t data   = {.output        = output,
                                  .database      = database,
                                  .users         = database_get_users(database),
                                  .flights       = database_get_flights(database),
                                  .commit_buffer = g_ptr_array_new(),
                                  .invalid_flight_ids =
                                      g_array_new(FALSE, FALSE, sizeof(flight_id_t)),
                                  .first_line = 1};
    int                 retval = 1;

    data.commit_buffer_id_pool = string_pool_create(PASSENGERS_LOADER_ID_POOL_BLOCK_CAPACITY);
    if (!data.commit_buffer_id_pool)
        goto DEFER_1;

    const fixed_n_delimiter_parser_iter_callback_t token_callbacks[2] = {
        __passengers_loader_parse_flight_id,
        __passengers_loader_parse_user_id};

    fixed_n_delimiter_parser_grammar_t *const line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 2, token_callbacks);
    if (!line_grammar)
        goto DEFER_2;

    dataset_parser_grammar_t *const grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __passengers_loader_before_parse_line,
                                   __passengers_loader_after_parse_line);
    if (!grammar)
        goto DEFER_3;

    retval = dataset_parser_parse(passengers_stream, grammar, &data);
    __passengers_loader_commit_flight_list(&data);
    __passengers_loader_report_erroneous_flights(&data, flights_stream);

    dataset_parser_grammar_free(grammar);
DEFER_3:
    fixed_n_delimiter_parser_grammar_free(line_grammar);
DEFER_2:
    string_pool_free(data.commit_buffer_id_pool);
DEFER_1:
    g_array_unref(data.invalid_flight_ids);
    g_ptr_array_unref(data.commit_buffer);

    return retval != 0;
}
