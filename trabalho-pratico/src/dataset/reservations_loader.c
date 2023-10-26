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
 * @file  reservations_loader.c
 * @brief Implementation of methods in include/dataset/reservations_loader.h
 */

#include <ctype.h>
#include <string.h>

#include "dataset/reservations_loader.h"
#include "utils/dataset_parser.h"

/** @brief Table header for `reservations_errors.csv` */
#define RESERVATIONS_LOADER_HEADER                                                                 \
    "id;user_id;hotel_id;hotel_name;hotel_stars;city_tax;address;begin_date;end_date;"             \
    "price_per_night;includes_breakfast;room_details;rating;comment"

/**
 * @struct reservations_loader_t
 * @brief  Temporary data needed to load a set of reservations.
 *
 * @var reservations_loader_t::dataset
 *     @brief Dataset loader, so that errors can be reported.
 * @var reservations_loader_t::database
 *     @brief Database in ::reservations_loader_t::dataset
 * @var reservations_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 */
typedef struct {
    dataset_loader_t *dataset;
    database_t       *database;

    char *error_line;
} reservations_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __reservations_loader_before_parse_line(void *loader_data, char *line) {
    ((reservations_loader_t *) loader_data)->error_line = line;
    return 0;
}

/* TODO - write values to a reservation field in reservations_loader_t */

/**
 * @brief Temporary function that reports parsing success.
 * @details To be replaced with actual parsing functions.
 */
int __reservations_loader_success(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) token;
    (void) ntoken;
    return 0;
}

/** @brief Places a parsed reservation in the database and handles errors */
int __reservations_loader_after_parse_line(void *loader_data, int retval) {
    if (retval) {
        reservations_loader_t *loader = (reservations_loader_t *) loader_data;
        dataset_loader_report_reservations_error(loader->dataset, loader->error_line);
    }
    return 0;
}

void reservations_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_reservations_error(dataset_loader, RESERVATIONS_LOADER_HEADER);
    reservations_loader_t data = {.dataset  = dataset_loader,
                                  .database = dataset_loader_get_database(dataset_loader)};

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[14] = {__reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success,
                                                                    __reservations_loader_success};

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 14, token_callbacks);

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __reservations_loader_before_parse_line,
                                   __reservations_loader_after_parse_line);

    dataset_parser_parse(stream, grammar, &data);
    dataset_parser_grammar_free(grammar);
}
