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
 * @file  q01.c
 * @brief Implementation of methods in include/queries/q01.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "queries/q01.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"
#include "utils/single_pool_id_linked_list.h"

/** @brief Type of the enitity queried by a query 1 */
typedef enum {
    ID_ENTITY_USER,       /**< @brief The queried entity is a user */
    ID_ENTITY_FLIGHT,     /**< @brief The queried entity is a flight */
    ID_ENTITY_RESERVATION /**< @brief The queried entity is a reservation */
} q01_id_entity_t;

/**
 * @struct q01_parsed_arguments_t
 * @brief  Data needed for the execution of a query of type 1.
 *
 * @var parse_arguments_helper::id_entity
 *     @brief Stores the type of the entity of an identifier.
 * @var parse_arguments_helper::parsed_id
 *     @brief A pointer to an id, of type `char *` or `uint64_t *`.
 */
typedef struct {
    q01_id_entity_t id_entity;
    void           *parsed_id;
} q01_parsed_arguments_t;

/**
 * @brief   Parses the arguments of a query of type 1.
 * @details Asserts there's only one string argument, that is stored.
 *
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 *
 * @return `NULL` for invalid arguments, a copy of the only @p argv and its identifier on success.
 */
void *__q01_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 1)
        return NULL;

    /* TODO - fix leaks */

    flight_id_t             parsed_flight_id_aux;
    reservation_id_t        parsed_reservation_id_aux;
    q01_parsed_arguments_t *parsed_argument = malloc(sizeof(q01_parsed_arguments_t));
    if (!parsed_argument)
        return NULL;

    if (!flight_id_from_string(&parsed_flight_id_aux, *argv)) {
        flight_id_t *parsed_id = malloc(sizeof(flight_id_t));
        if (!parsed_id)
            return NULL;

        *parsed_id                 = parsed_flight_id_aux;
        parsed_argument->id_entity = ID_ENTITY_FLIGHT;
        parsed_argument->parsed_id = parsed_id;
    } else if (!reservation_id_from_string(&parsed_reservation_id_aux, *argv)) {
        reservation_id_t *parsed_id = malloc(sizeof(reservation_id_t));
        if (!parsed_id)
            return NULL;

        *parsed_id                 = parsed_reservation_id_aux;
        parsed_argument->id_entity = ID_ENTITY_RESERVATION;
        parsed_argument->parsed_id = parsed_id;
    } else {
        char *parsed_id = strdup(*argv);
        if (!parsed_id)
            return NULL;

        parsed_argument->id_entity = ID_ENTITY_USER;
        parsed_argument->parsed_id = parsed_id;
    }

    return parsed_argument;
}

void *__q01_clone_arguments(const void *args_data) {
    const q01_parsed_arguments_t *args  = args_data;
    q01_parsed_arguments_t       *clone = malloc(sizeof(q01_parsed_arguments_t));
    if (!clone)
        return NULL;

    clone->id_entity = args->id_entity;
    switch (args->id_entity) {
        case ID_ENTITY_FLIGHT:
            clone->parsed_id = malloc(sizeof(sizeof(flight_id_t)));
            memcpy(clone->parsed_id, args->parsed_id, sizeof(flight_id_t));
            break;
        case ID_ENTITY_RESERVATION:
            clone->parsed_id = malloc(sizeof(sizeof(reservation_id_t)));
            memcpy(clone->parsed_id, args->parsed_id, sizeof(reservation_id_t));
            break;
        case ID_ENTITY_USER:
            clone->parsed_id = strdup(args->parsed_id);
            break;
        default:
            clone->parsed_id = NULL; /* unreachable */
            break;
    }

    if (!clone->parsed_id) {
        free(clone);
        return NULL;
    }
    return clone;
}

/**
 * @brief Frees data generated by ::__q01_parse_arguments.
 * @param argument_data Data generated by ::__q01_parse_arguments.
 */
void __q01_free_query_instance_argument_data(void *argument_data) {
    free(((q01_parsed_arguments_t *) argument_data)->parsed_id);
    free(argument_data);
}

/**
 * @brief   Calculates the total spent by a ::user_t.
 * @details The total spent is the sum of the total price of all the reservations the user is
 *          related to.
 *
 * @param list    List to get the reservation identifiers from.
 * @param manager Manager to get the reservations from.
 *
 * @return The total spent by a ::user_t.
 */
double __q01_calculate_user_total_spent(const single_pool_id_linked_list_t *list,
                                        const reservation_manager_t        *manager) {
    double total_spent = 0.0;

    while (list) {
        const reservation_t *reservation =
            reservation_manager_get_by_id(manager, single_pool_id_linked_list_get_value(list));
        total_spent += reservation_calculate_price(reservation);
        list = single_pool_id_linked_list_get_next(list);
    }

    return total_spent;
}

/**
 * @brief Executes a query of type 1, when it refers to a ::user_t.
 *
 * @param database Database do get the users and reservations from.
 * @param id       Identifier to find the user and its data.
 * @param output   Where to write the query's output to.
 *
 * @retval 0 Never fails.
 */
int __q01_execute_user_entity(const database_t *database, const char *id, query_writer_t *output) {

    const user_manager_t        *user_manager        = database_get_users(database);
    const reservation_manager_t *reservation_manager = database_get_reservations(database);
    const user_t                *user                = user_manager_get_by_id(user_manager, id);
    if (!user || user_get_account_status(user) == ACCOUNT_STATUS_INACTIVE)
        return 0;

    const single_pool_id_linked_list_t *reservation_list =
        user_manager_get_reservations_by_id(user_manager, id);
    size_t number_of_flights =
        single_pool_id_linked_list_length(user_manager_get_flights_by_id(user_manager, id));
    size_t number_of_reservations = single_pool_id_linked_list_length(reservation_list);

    double total_spent = __q01_calculate_user_total_spent(reservation_list, reservation_manager);

    char sex[SEX_SPRINTF_MIN_BUFFER_SIZE];
    sex_sprintf(sex, user_get_sex(user));

    int32_t age = user_calculate_age(user);

    char country_code[COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE];
    country_code_sprintf(country_code, user_get_country_code(user));

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "name", "%s", user_get_const_name(user));
    query_writer_write_new_field(output, "sex", "%s", sex);
    query_writer_write_new_field(output, "age", "%" PRIi32, age);
    query_writer_write_new_field(output, "country_code", "%s", country_code);
    query_writer_write_new_field(output, "passport", "%s", user_get_const_passport(user));
    query_writer_write_new_field(output, "number_of_flights", "%zu", number_of_flights);
    query_writer_write_new_field(output, "number_of_reservations", "%zu", number_of_reservations);
    query_writer_write_new_field(output, "total_spent", "%.3lf", total_spent);

    return 0;
}

/**
 * @brief Executes a query of type 1, when it refers to a ::reservation_t.
 *
 * @param database Database do get the users and reservations from.
 * @param id       Identifier to find the reservation and its data.
 * @param output   Where to write the query's output to.
 *
 * @retval 0 Always successful.
 */
int __q01_execute_reservation_entity(const database_t *database,
                                     reservation_id_t  id,
                                     query_writer_t   *output) {

    const reservation_manager_t *reservation_manager = database_get_reservations(database);
    const reservation_t *reservation = reservation_manager_get_by_id(reservation_manager, id);
    if (!reservation)
        return 0;

    date_t begin_date = reservation_get_begin_date(reservation);
    char   begin_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(begin_date_str, begin_date);

    date_t end_date = reservation_get_end_date(reservation);
    char   end_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(end_date_str, end_date);

    includes_breakfast_t includes_breakfast = reservation_get_includes_breakfast(reservation);
    char                 includes_breakfast_str[INCLUDES_BREAKFAST_SPRINTF_MIN_BUFFER_SIZE];
    includes_breakfast_sprintf(includes_breakfast_str, includes_breakfast);

    int64_t nights      = date_diff(end_date, begin_date);
    double  total_price = reservation_calculate_price(reservation);

    char hotel_id_str[HOTEL_ID_SPRINTF_MIN_BUFFER_SIZE];
    hotel_id_sprintf(hotel_id_str, reservation_get_hotel_id(reservation));

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "hotel_id", "%s", hotel_id_str);
    query_writer_write_new_field(output,
                                 "hotel_name",
                                 "%s",
                                 reservation_get_const_hotel_name(reservation));
    query_writer_write_new_field(output,
                                 "hotel_stars",
                                 "%" PRIu8,
                                 reservation_get_hotel_stars(reservation));
    query_writer_write_new_field(output, "begin_date", "%s", begin_date_str);
    query_writer_write_new_field(output, "end_date", "%s", end_date_str);
    query_writer_write_new_field(output, "includes_breakfast", "%s", includes_breakfast_str);
    query_writer_write_new_field(output, "nights", "%" PRIi64, nights);
    query_writer_write_new_field(output, "total_price", "%.3lf", total_price);

    return 0;
}

/**
 * @brief Executes a query of type 1, when it refers to a ::flight_t.
 *
 * @param database Database do get the flights from.
 * @param id       Id to find the flight.
 * @param output   Where to write the query's output to.
 *
 * @retval 0 Always successful.
 */
int __q01_execute_flight_entity(const database_t *database,
                                flight_id_t       id,
                                query_writer_t   *output) {

    const flight_manager_t *flight_manager = database_get_flights(database);
    const flight_t         *flight         = flight_manager_get_by_id(flight_manager, id);
    if (!flight)
        return 0;

    char origin_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(origin_airport, flight_get_origin(flight));

    char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(destination_airport, flight_get_destination(flight));

    date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);
    char            scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(scheduled_departure_str, schedule_departure_date);

    char scheduled_arrival_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(scheduled_arrival_str, flight_get_schedule_arrival_date(flight));

    int64_t delay =
        date_and_time_diff(flight_get_real_departure_date(flight), schedule_departure_date);

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "airline", "%s", flight_get_const_airline(flight));
    query_writer_write_new_field(output, "plane_model", "%s", flight_get_const_plane_model(flight));
    query_writer_write_new_field(output, "origin", "%s", origin_airport);
    query_writer_write_new_field(output, "destination", "%s", destination_airport);
    query_writer_write_new_field(output, "schedule_departure_date", "%s", scheduled_departure_str);
    query_writer_write_new_field(output, "schedule_arrival_date", "%s", scheduled_arrival_str);
    query_writer_write_new_field(output,
                                 "passengers",
                                 "%" PRIu16,
                                 flight_get_number_of_passengers(flight));
    query_writer_write_new_field(output, "delay", "%" PRIi64, delay);

    return 0;
}

/**
 * @brief Executes a query of type 1.
 *
 * @param database   Database where to get users/reservations/flights from
 * @param statistics `NULL`, as this query does not generate statistical data.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 On success.
 * @retval 1 On failure.
 */
int __q01_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) statistics;

    const q01_parsed_arguments_t *arguments = query_instance_get_argument_data(instance);
    const void                   *id        = arguments->parsed_id;

    switch (arguments->id_entity) {
        case ID_ENTITY_USER:
            return __q01_execute_user_entity(database, (const char *) id, output);
        case ID_ENTITY_RESERVATION:
            return __q01_execute_reservation_entity(database, *(reservation_id_t *) id, output);
        case ID_ENTITY_FLIGHT:
            return __q01_execute_flight_entity(database, *(flight_id_t *) id, output);
        default:
            return 1; /* unreachable */
    }
}

query_type_t *q01_create(void) {
    return query_type_create(1,
                             __q01_parse_arguments,
                             __q01_clone_arguments,
                             __q01_free_query_instance_argument_data,
                             NULL,
                             NULL,
                             __q01_execute);
}
