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
 *
 * @details Many internal methods in this module are lacking parameter documentation, as they all
 *          follow the same convention: all are ::fixed_n_delimiter_parser_iter_callback_t. The
 *          `loader_data` is a pointer to a ::reservations_loader_t.
 */

#include <string.h>

#include "dataset/dataset_parser.h"
#include "dataset/reservations_loader.h"
#include "utils/int_utils.h"

/** @cond FALSE */
#ifndef __GNUC__
    #define __builtin_unreachable() return 0;
#endif
/** @endcond */

/**
 * @struct reservations_loader_t
 * @brief  Temporary data needed to load a set of reservations.
 *
 * @var reservations_loader_t::output
 *     @brief Where to output dataset errors to.
 * @var reservations_loader_t::database
 *     @brief Database to add new reservations to.
 * @var reservations_loader_t::users
 *     @brief User manager, to check for the existence of users mentioned in reservations.
 * @var reservations_loader_t::error_line
 *     @brief Current line being processed, in case it needs to be put in the errors file.
 * @var reservations_loader_t::current_reservation
 *     @brief Reservation being currently parsed, whose fields are still being filled in.
 * @var reservations_loader_t::first_line
 *     @brief   Whether the line being parsed is the first line in the file.
 *     @details Used to print an error on the CSV's table header.
 */
typedef struct {
    dataset_error_output_t *const output;
    database_t *const             database;
    const user_manager_t *const   users;

    const char          *error_line;
    reservation_t *const current_reservation;
    int                  first_line;
} reservations_loader_t;

/**
 * @brief Stores the beginning of the current line, in case it needs to be printed to the errors
 *        file.
 *
 * @param loader_data A pointer to a ::reservations_loader_t.
 * @param line        Line that is going to be parsed.
 *
 * @retval 0 Always successful.
 */
int __reservations_loader_before_parse_line(void *loader_data, char *line) {
    ((reservations_loader_t *) loader_data)->error_line = line;
    return 0;
}

/** @brief Parses a reservation's identifier. */
int __reservation_loader_parse_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *const loader = loader_data;

    reservation_id_t id;
    const int        retval = reservation_id_from_string(&id, token);
    if (retval == 0) {
        reservation_set_id(loader->current_reservation, id);
    } else if (retval == 2 && !(loader->first_line && strcmp(token, "id") == 0)) {
        fprintf(stderr,
                "Reservation ID \"%s\" not if format BookXXXXXXXXXX. This isn't supported by our "
                "program!\n",
                token);
    }
    return retval;
}

/** @brief Parses the identifier of the user that booked a reservation. */
int __reservation_loader_parse_user_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *const loader = loader_data;

    if (*token && user_manager_get_by_id(loader->users, token) != NULL)
        return reservation_set_user_id(NULL, loader->current_reservation, token);
    else
        return 1;
}

/** @brief Parses the identifier of the hotel of a reservation. */
int __reservation_loader_parse_hotel_id(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    reservations_loader_t *const loader = loader_data;

    hotel_id_t id;
    const int  retval = hotel_id_from_string(&id, token);
    if (retval == 0) {
        reservation_set_hotel_id(loader->current_reservation, id);
    } else if (retval == 2 && !(loader->first_line && strcmp(token, "hotel_id") == 0)) {
        fprintf(stderr,
                "Hotel ID \"%s\" not if format HTLXXXXX. This isn't supported by our program!\n",
                token);
    }
    return retval;
}

/** @brief Parses a reservation's hotel name. */
int __reservation_loader_parse_hotel_name(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;
    return reservation_set_hotel_name(NULL,
                                      ((reservations_loader_t *) loader_data)->current_reservation,
                                      token);
}

/**
 * @brief   Parses numbers in hotel reservations.
 * @details This includes hotel stars, city tax and prices per night. Because ratings can be left
 *          empty, they require a different handler function.
 */
int __reservation_loader_parse_mandatory_numeral(void *loader_data, char *token, size_t ntoken) {
    reservations_loader_t *const loader = loader_data;

    uint64_t  numeral;
    const int retval = int_utils_parse_positive(&numeral, token);
    if (retval)
        return retval;

    switch (ntoken) {
        case 4:
            return reservation_set_hotel_stars(loader->current_reservation, numeral);
        case 5:
            reservation_set_city_tax(loader->current_reservation, numeral);
            return 0;
        case 9:
            return reservation_set_price_per_night(loader->current_reservation, numeral);
        default:
            __builtin_unreachable();
    }
}

/** @brief Parses a reservation's hotel address. */
int __reservation_loader_parse_address(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) ntoken;
    return (*token == '\0'); /* Fail on empty addresses */
}

/** @brief Parses a reservation's beginning and end dates. */
int __reservation_loader_parse_date(void *loader_data, char *token, size_t ntoken) {
    reservations_loader_t *const loader = loader_data;

    date_t    date;
    const int retval = date_from_string(&date, token);
    if (retval)
        return retval;

    switch (ntoken) {
        case 7:
            return reservation_set_begin_date(loader->current_reservation, date);
        case 8:
            return reservation_set_end_date(loader->current_reservation, date);
        default:
            __builtin_unreachable();
    }
}

/** @brief Parses a reservation's "includes breakfast" field. */
int __reservation_loader_parse_includes_breakfast(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    includes_breakfast_t includes_breakfast;
    const int            retval = includes_breakfast_from_string(&includes_breakfast, token);
    if (retval)
        return retval;

    reservation_set_includes_breakfast(((reservations_loader_t *) loader_data)->current_reservation,
                                       includes_breakfast);
    return 0;
}

/**
 * @brief   Parses a reservation's room details and comment fields.
 * @details These are strings that do not require verification.
 */
int __reservation_loader_parse_dont_verify(void *loader_data, char *token, size_t ntoken) {
    (void) loader_data;
    (void) token;
    (void) ntoken;
    return 0;
}

/** @brief Parses a reservation's rating. */
int __reservation_loader_parse_rating(void *loader_data, char *token, size_t ntoken) {
    (void) ntoken;

    uint64_t  rating = RESERVATION_NO_RATING;
    const int retval = int_utils_parse_positive(&rating, token);
    if (retval && *token)
        return retval; /* Invalid number */

    return reservation_set_rating(((reservations_loader_t *) loader_data)->current_reservation,
                                  rating);
}

/**
 * @brief Places a parsed reservation in the database and handles errors.
 *
 * @param loader_data A pointer to a ::reservations_loader_t.
 * @param retval      Value returned by the last token callback (`0` for success, another value for
 *                    a parsing error).
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __reservations_loader_after_parse_line(void *loader_data, int retval) {
    reservations_loader_t *const loader = loader_data;
    loader->first_line                  = 0;

    if (retval) {
        dataset_error_output_report_reservation_error(loader->output, loader->error_line);
        retval = 0;
    } else {
        retval = database_add_reservation(loader->database, loader->current_reservation);
    }

    reservation_reset_dates(loader->current_reservation);
    return retval;
}

int reservations_loader_load(FILE *stream, database_t *database, dataset_error_output_t *output) {
    reservations_loader_t data = {.output              = output,
                                  .database            = database,
                                  .users               = database_get_users(database),
                                  .current_reservation = reservation_create(NULL),
                                  .first_line          = 1};
    if (!data.current_reservation)
        return 1;

    const fixed_n_delimiter_parser_iter_callback_t token_callbacks[14] = {
        __reservation_loader_parse_id,
        __reservation_loader_parse_user_id,
        __reservation_loader_parse_hotel_id,
        __reservation_loader_parse_hotel_name,
        __reservation_loader_parse_mandatory_numeral,
        __reservation_loader_parse_mandatory_numeral,
        __reservation_loader_parse_address,
        __reservation_loader_parse_date,
        __reservation_loader_parse_date,
        __reservation_loader_parse_mandatory_numeral,
        __reservation_loader_parse_includes_breakfast,
        __reservation_loader_parse_dont_verify,
        __reservation_loader_parse_rating,
        __reservation_loader_parse_dont_verify};

    fixed_n_delimiter_parser_grammar_t *const line_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 14, token_callbacks);
    if (!line_grammar) {
        reservation_free(data.current_reservation);
        return 1;
    }

    dataset_parser_grammar_t *const grammar =
        dataset_parser_grammar_new('\n',
                                   line_grammar,
                                   __reservations_loader_before_parse_line,
                                   __reservations_loader_after_parse_line);
    if (!grammar) {
        fixed_n_delimiter_parser_grammar_free(line_grammar);
        reservation_free(data.current_reservation);
        return 1;
    }

    const int retval = dataset_parser_parse(stream, grammar, &data);

    fixed_n_delimiter_parser_grammar_free(line_grammar);
    dataset_parser_grammar_free(grammar);
    reservation_free(data.current_reservation);

    return retval != 0;
}
