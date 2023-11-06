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
#include <stdlib.h>

#include "dataset/reservations_loader.h"
#include "utils/dataset_parser.h"
#include "types/reservation.h"
#include "types/includes_breakfast.h"

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

    reservation_t *current_reservation;
    char          *user_id_terminator, *hotel_name_terminator;
} reservations_loader_t;

#define BOOK 4
#define HTL 3
#define NO_RATING -1

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __reservations_loader_before_parse_line(void *loader_data, char *line) {
    ((reservations_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a reservation's identifier */
int __reservation_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_id(loader->current_reservation, atoi(token + BOOK));
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's user identifier */
int __reservation_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_user_id(loader->current_reservation, token);
        loader->user_id_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's hotel id */
int __reservation_loader_parse_hotel_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_hotel_id(loader->current_reservation, atoi(token + HTL));
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's hotel name */
int __reservation_loader_parse_hotel_name(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_hotel_name(loader->current_reservation, token);
        loader->hotel_name_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's hotel stars */
int __reservation_loader_parse_hotel_stars(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    double token_number_value = atoi (token);

    if (!(token_number_value - (int)token_number_value)
        && token_number_value <= 5
        && token_number_value >= 1) {
        reservation_set_hotel_stars(loader->current_reservation, token_number_value);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's city tax */
int __reservation_loader_parse_city_tax(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    double token_number_value = atoi (token);

    if (!(token_number_value - (int)token_number_value)
        && token_number_value >= 0) {
        reservation_set_city_tax(loader->current_reservation, token_number_value);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's address */
int __reservation_loader_parse_address(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return (*token == '\0'); /* Fail on empty addresses */
}

/** @brief Parses a reservation's begin date */
int __reservation_loader_parse_begin_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    date_t date;
    int    date_parse_ret = date_from_string(&date, token);
    if (date_parse_ret) {
        return date_parse_ret;
    } else {
        reservation_set_begin_date(loader->current_reservation, date);
        return 0;
    }
}

/** @brief Parses a reservation's end date */
int __reservation_loader_parse_end_date(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    date_t date;
    int    date_parse_ret = date_from_string(&date, token);
    if (date_parse_ret) {
        return date_parse_ret;
    } else {
        reservation_set_end_date(loader->current_reservation, date);
        return 0;
    }
}

/** @brief Parses a reservation's price per night */
int __reservation_loader_parse_price_per_night(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    double token_number_value = atoi (token);

    if (!(token_number_value - (int)token_number_value)
        && token_number_value >= 0) {
        reservation_set_price_per_night(loader->current_reservation, token_number_value);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's "includes breakfast" tag */
int __reservation_loader_parse_includes_breakfast(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    
    includes_breakfast_t *includes_breakfast = NULL;
    int includes_breakfast_ret = includes_breakfast_from_string(includes_breakfast, token);

    if (!includes_breakfast_ret) {
        reservation_set_includes_breakfast(loader->current_reservation, *includes_breakfast);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's room details */
int __reservation_loader_parse_room_details(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) token;
    (void) ntoken;

    return 0; /* no verification needed */
}

/** @brief Parses a reservation's rating */
int __reservation_loader_parse_rating(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    if (*token == '\0') {
        reservation_set_price_per_night(loader->current_reservation, NO_RATING);
        return 0;
    }

    double token_number_value = atoi (token);

    if (!(token_number_value - (int)token_number_value)
        && token_number_value >= 1
        && token_number_value <= 5) {
        reservation_set_price_per_night(loader->current_reservation, token_number_value);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's comment */
int __reservation_loader_parse_comment(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) token;
    (void) ntoken;

    return 0; /* no verification needed */
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

    fixed_n_delimiter_parser_iter_callback_t
                                token_callbacks[14]={__reservation_loader_parse_id,
                                                     __reservation_loader_parse_user_id,
                                                     __reservation_loader_parse_hotel_id,
                                                     __reservation_loader_parse_hotel_name,
                                                     __reservation_loader_parse_hotel_stars,
                                                     __reservation_loader_parse_city_tax,
                                                     __reservation_loader_parse_address,
                                                     __reservation_loader_parse_begin_date,
                                                     __reservation_loader_parse_end_date,
                                                     __reservation_loader_parse_price_per_night,
                                                     __reservation_loader_parse_includes_breakfast,
                                                     __reservation_loader_parse_room_details,
                                                     __reservation_loader_parse_rating,
                                                     __reservation_loader_parse_comment};

    fixed_n_delimiter_parser_grammar_t *line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 14, token_callbacks);

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __reservations_loader_before_parse_line,
                                   __reservations_loader_after_parse_line);
    dataset_parser_parse(stream, grammar, &data);
}
