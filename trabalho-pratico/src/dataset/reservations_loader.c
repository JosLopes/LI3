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
#include <stdlib.h>
#include <string.h>

#include "dataset/reservations_loader.h"
#include "database/reservation_manager.h"
#include "types/includes_breakfast.h"
#include "types/reservation.h"
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
    dataset_loader_t      *dataset;
    reservation_manager_t *reservations;

    char *error_line;

    reservation_t *current_reservation;
    char          *user_id_terminator, *hotel_name_terminator;
} reservations_loader_t;

/* Verifies if a string only contains zero's */
int __all_zeros_string(char *token) {
    for (int index = 0; token[index] != '\0'; index++) {
        if (token[index] != '0')
            return 1;
    }
    return 0;
}

/**
 * @brief Verifies if a string corresponds to a whole number.
 *
 * @details If the string corresponds to an whole number, the value is stored on the @p output.
 *
 * @retval 0 True.
 * @retval 1 False.
 */
int __its_an_whole_number(int *output, char *token) {
    int index;
    int string_sum = 0;
    for (index = 0; token[index] != '\0' && token[index] != '.'; index++) {
        string_sum += string_sum * 10 + (token[index] - 48);
    }

    if ((token[index] == '.' && __all_zeros_string(token + index + 1))) {
        return 1;
    }

    *output = string_sum;
    return 0;
}

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 */
int __reservations_loader_before_parse_line(void *loader_data, char *line) {
    ((reservations_loader_t *) loader_data)->error_line = line;
    return 0;
}

/* Before the numerical part of an identifier, there is an "BOOK" string */
#define NUMBER_OF_CHARACTERS_BEFORE_INT_BOOK 4

/** @brief Parses a reservation's identifier */
int __reservation_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_id(loader->current_reservation,
                           atoi(token + NUMBER_OF_CHARACTERS_BEFORE_INT_BOOK));
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's user identifier */
int __reservation_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    user_manager_t *user_manager = database_get_users(dataset_loader_get_database(loader->dataset));

    size_t length = strlen(token);
    /* TODO - verify if this condition is needed
     * user_manager_get_by_id(user_manager, token)!=NULL
    */
    (void) user_manager;
    if (length) {
        reservation_set_user_id(loader->current_reservation, token);
        loader->user_id_terminator = token + length;
        return 0;
    } else {
        return 1;
    }
}

/* Before the numerical part of an hotel identifier, there is an "HTL" string */
#define NUMBER_OF_CHARACTERS_BEFORE_INT_HOTEL_ID 3

/** @brief Parses a reservation's hotel id */
int __reservation_loader_parse_hotel_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    size_t length = strlen(token);
    if (length) {
        reservation_set_hotel_id(loader->current_reservation,
                                 atoi(token + NUMBER_OF_CHARACTERS_BEFORE_INT_HOTEL_ID));
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

#define STARS_CELLING_VALUE 5
#define STARS_FLOOR_VALUE   1

/** @brief Parses a reservation's hotel stars */
int __reservation_loader_parse_hotel_stars(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    int                    output;
    int                    whole_number_ret = __its_an_whole_number(&output, token);

    if (whole_number_ret == 0 && output >= STARS_FLOOR_VALUE &&
        output <= STARS_CELLING_VALUE) {
        reservation_set_hotel_stars(loader->current_reservation, output);
        return 0;
    } else {
        return 1;
    }
}

#define CITY_TAX_FLOOR_VALUE 0

/** @brief Parses a reservation's city tax */
int __reservation_loader_parse_city_tax(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    int                    output;
    int                    whole_number_ret = __its_an_whole_number(&output, token);

    if (whole_number_ret == 0 && output >= CITY_TAX_FLOOR_VALUE) {
        reservation_set_city_tax(loader->current_reservation, output);
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

#define PRICE_PER_NIGHT_FLOOR_VALUE 1

/** @brief Parses a reservation's price per night */
int __reservation_loader_parse_price_per_night(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;
    int                    output;
    int                    whole_number_ret = __its_an_whole_number(&output, token);

    if (whole_number_ret == 0 && output >= PRICE_PER_NIGHT_FLOOR_VALUE) {
        reservation_set_price_per_night(loader->current_reservation, output);
        return 0;
    } else {
        return 1;
    }
}

/** @brief Parses a reservation's "includes breakfast" tag */
int __reservation_loader_parse_includes_breakfast(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    includes_breakfast_t *includes_breakfast = malloc(sizeof(enum includes_breakfast));
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

#define NO_RATING            -1 /* Value to return when no rating was given to a reservation */
#define RATING_CELLING_VALUE 5
#define RATING_FLOOR_VALUE   1

/** @brief Parses a reservation's rating */
int __reservation_loader_parse_rating(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *loader = (reservations_loader_t *) loader_data;

    if (*token == '\0') {
        reservation_set_rating(loader->current_reservation, NO_RATING);
        return 0;
    }

    int output;
    int whole_number_ret = __its_an_whole_number(&output, token);

    if (whole_number_ret == 0 && output >= RATING_FLOOR_VALUE &&
        output <= RATING_CELLING_VALUE) {
        reservation_set_rating(loader->current_reservation, output);
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

        reservation_manager_add_reservation(loader->reservations, loader->current_reservation);
    }
    return 0;
}

void reservations_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    dataset_loader_report_reservations_error(dataset_loader, RESERVATIONS_LOADER_HEADER);
    reservations_loader_t data = {
        .dataset      = dataset_loader,
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

    dataset_parser_grammar_t *grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __reservations_loader_before_parse_line,
                                   __reservations_loader_after_parse_line);
    dataset_parser_parse(stream, grammar, &data);
}
