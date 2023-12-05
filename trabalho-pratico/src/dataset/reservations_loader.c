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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database/reservation_manager.h"
#include "dataset/dataset_parser.h"
#include "dataset/reservations_loader.h"
#include "types/reservation.h"
#include "utils/int_utils.h"

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
 * @var reservations_loader_t::users
 *     @brief Users manager to check for existence of users mentioned in reservations.
 * @var reservations_loader_t::reservations
 *     @brief Reservations manager to add new reservations to.
 * @var reservations_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the error file.
 * @var reservations_loader_t::user_id_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that the user's identifier ends.
 * @var reservations_loader_t::hotel_name_terminator
 *     @brief Where a ``'\0'`` terminator needs to be placed, so that a reservation's hotel name
 *            ends.
 */
typedef struct {
    dataset_loader_t      *dataset;
    user_manager_t        *users;
    reservation_manager_t *reservations;

    char *error_line;

    reservation_t *current_reservation;
    char          *user_id_terminator, *hotel_name_terminator;
} reservations_loader_t;

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
    if (length > 4) { /* Skip "Book" before any reservation ID */
        uint64_t id;
        int      retval = int_utils_parse_positive(&id, token + 4);

        if (retval) {
            fprintf(stderr,
                    "Non-numerical reservation ID detected, \"%s\". Our program's architecture "
                    "doesn't allow for this.\n",
                    token);
            return 1;
        } else {
            reservation_set_id(loader->current_reservation, id);
            return 0;
        }
    } else {
        if (length != 0) {
            fprintf(stderr,
                    "Reservation ID detected that doesn't start with \"Book\" detected, \"%s\". "
                    "Our program's architecture doesn't allow for this.\n",
                    token);
        }

        return 1;
    }
}

/** @brief Parses a reservation's user identifier */
int __reservation_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length && user_manager_get_by_id(loader->users, token) != NULL) {
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
    if (length > 3) { /* Skip "HTL" before any reservation ID */
        uint64_t id;
        int      retval = int_utils_parse_positive(&id, token + 3);

        if (retval) {
            fprintf(stderr,
                    "Non-numerical hotel ID detected, \"%s\". Our program's architecture doesn't "
                    "doesn't allow for this.\n",
                    token);
            return 1;
        } else {
            reservation_set_hotel_id(loader->current_reservation, id);
            return 0;
        }
    } else {
        if (length != 0) {
            fprintf(stderr,
                    "Hotel ID detected that doesn't start with \"HTL\" detected, \"%s\". "
                    "Our program's architecture doesn't allow for this.\n",
                    token);
        }

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

/** @brief Parses a reservation's hotel number of stars */
int __reservation_loader_parse_hotel_stars(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    uint64_t hotel_stars;
    int      retval = int_utils_parse_positive(&hotel_stars, token);

    if (retval == 0 && hotel_stars >= 1 && hotel_stars <= 5) {
        reservation_set_hotel_stars(loader->current_reservation, hotel_stars);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's hotel city tax */
int __reservation_loader_parse_city_tax(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    uint64_t city_tax;
    int      retval = int_utils_parse_positive(&city_tax, token);

    if (!retval) {
        reservation_set_city_tax(loader->current_reservation, city_tax);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's hotel address */
int __reservation_loader_parse_address(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;

    return (*token == '\0'); /* Fail on empty addresses */
}

/** @brief Parses a reservation's beginning date */
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
    } else if (date_diff(date, reservation_get_begin_date(loader->current_reservation)) < 0) {
        return 1;
    } else {
        reservation_set_end_date(loader->current_reservation, date);
        return 0;
    }
}

/** @brief Parses a reservation's price per night */
int __reservation_loader_parse_price_per_night(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    uint64_t price_per_night;
    int      retval = int_utils_parse_positive(&price_per_night, token);

    if (retval == 0 && price_per_night > 0) {
        reservation_set_price_per_night(loader->current_reservation, price_per_night);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's "includes breakfast" tag */
int __reservation_loader_parse_includes_breakfast(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    includes_breakfast_t includes_breakfast;
    int includes_breakfast_ret = includes_breakfast_from_string(&includes_breakfast, token);

    if (!includes_breakfast_ret) {
        reservation_set_includes_breakfast(loader->current_reservation, includes_breakfast);
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
        reservation_set_rating(loader->current_reservation, RESERVATION_NO_RATING);
        return 0;
    }

    uint64_t rating;
    int      retval = int_utils_parse_positive(&rating, token);

    if (retval == 0 && rating >= 1 && rating <= 5) {
        reservation_set_rating(loader->current_reservation, rating);
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
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    if (retval) {
        dataset_loader_report_reservations_error(loader->dataset, loader->error_line);
    } else {
        /* Restore token terminations for strings that will be stored in the reservation. */
        *loader->user_id_terminator    = '\0';
        *loader->hotel_name_terminator = '\0';

        /* Ignore allocation errors */
        reservation_manager_add_reservation(loader->reservations, loader->current_reservation);
        user_manager_add_user_reservation_association(
            loader->users,
            reservation_get_const_user_id(loader->current_reservation),
            reservation_get_id(loader->current_reservation));
    }
    return 0;
}

void reservations_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_reservations_error(dataset_loader, RESERVATIONS_LOADER_HEADER);
    reservations_loader_t data = {
        .dataset      = dataset_loader,
        .users        = database_get_users(dataset_loader_get_database(dataset_loader)),
        .reservations = database_get_reservations(dataset_loader_get_database(dataset_loader)),
        .current_reservation = reservation_create()};

    fixed_n_delimiter_parser_iter_callback_t token_callbacks[14] = {
        __reservation_loader_parse_id,
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
    if (!line_grammar) {
        reservation_free(data.current_reservation);
        return;
    }

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __reservations_loader_before_parse_line,
                                   __reservations_loader_after_parse_line);
    if (!grammar) {
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        reservation_free(data.current_reservation);
        return;
    }

    dataset_parser_parse(stream, grammar, &data);

    fixed_n_delimiter_parser_grammar_free(line_grammar);
    dataset_parser_grammar_free(grammar);
    reservation_free(data.current_reservation);
}
