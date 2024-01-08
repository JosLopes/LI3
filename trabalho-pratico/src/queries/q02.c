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
 * @file  q02.c
 * @brief Implementation of methods in include/queries/q02.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queries/q02.h"
#include "queries/query_instance.h"

/** @brief Argument values of the query of type 2. */
typedef enum {
    Q02_ARGUMENTS_NO_ARGUMENT, /**< Display both flights and reservations. */
    Q02_ARGUMENTS_FLIGHTS,     /**< List only flights. */
    Q02_ARGUMENTS_RESERVATIONS /**< List only reservations. */
} q02_arguments_output_filter_t;

/**
 * @struct q02_argument_data_t
 * @brief  Parsed arguments of a query of type 2
 *
 * @var q02_argument_data_t::user_id
 *     @brief User whose reservations / flights (or both) should be consulted.
 * @var q02_argument_data_t::filter
 *     @brief What the query requires to be outputted (reservations, flights, or both).
 */
typedef struct {
    char                         *user_id;
    q02_arguments_output_filter_t filter;
} q02_argument_data_t;

/**
 * @brief   Parses arguments for the second query type.
 * @details The first argument, a user identifier, is mandatory. A optional second argument, taking
 *          the value of either `"flights"` or `"reservations"`, is allowed.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` on failure, a pointer to a `q02_argument_data_t` otherwise.
 */
void *__q02_parse_arguments(char **argv, size_t argc) {
    if (argc == 1) {
        q02_argument_data_t *ret = malloc(sizeof(q02_argument_data_t));
        ret->user_id             = strdup(argv[0]);
        ret->filter              = Q02_ARGUMENTS_NO_ARGUMENT;
        return ret;
    } else if (argc == 2) {
        q02_arguments_output_filter_t filter;

        if (strcmp(argv[1], "flights") == 0)
            filter = Q02_ARGUMENTS_FLIGHTS;
        else if (strcmp(argv[1], "reservations") == 0)
            filter = Q02_ARGUMENTS_RESERVATIONS;
        else
            return NULL;

        q02_argument_data_t *ret = malloc(sizeof(q02_argument_data_t));
        ret->user_id             = strdup(argv[0]);
        ret->filter              = filter;
        return ret;
    }

    return NULL;
}

/**
 * @brief Frees argument data for query 2, allocated by ::__q02_parse_arguments.
 * @param argument_data Data allocated by ::__q02_parse_arguments.
 */
void __q02_free_query_instance_argument_data(void *argument_data) {
    free(((q02_argument_data_t *) argument_data)->user_id);
    free(argument_data);
}

/**
 * @struct q02_output_item_t
 * @brief  One item of this query's output (a flight or reservation).
 *
 * @var q02_output_item_t::id
 *     @brief Identifier of the object in question (flight or reservation).
 * @var q02_output_item_t::date
 *     @brief Date and time the object in question happened in.
 * @var q02_output_item_t::type
 *     @brief Type of the object in question (flight or reservation).
 */
typedef struct {
    uint32_t        id;
    date_and_time_t date;

    enum {
        Q02_OUTPUT_ITEM_FLIGHT,     /**< This output item is a flight. */
        Q02_OUTPUT_ITEM_RESERVATION /**< This output item is a reservation. */
    } type;
} q02_output_item_t;

/**
 * @brief   Comparison function for sorting the output of query 2.
 * @details Auxiliary function for ::__q02_execute.
 */
gint __q02_execute_sort_compare(gconstpointer a_data, gconstpointer b_data) {
    const q02_output_item_t *a = (const q02_output_item_t *) a_data;
    const q02_output_item_t *b = (const q02_output_item_t *) b_data;

    int64_t crit1 = date_and_time_diff(b->date, a->date);
    if (crit1)
        return crit1;

    int64_t crit3 = (int32_t) a->id - (int32_t) b->id;
    return crit3;
}

/**
 * @brief Prints the output of query 2 to an @p output file.
 *
 * @param output    Where to output the query's results to.
 * @param items     Array of ::q02_output_item_t, to be written as text.
 * @param filter    Whether the query requested flights, reservations or both.
 */
void __q02_print_output(query_writer_t               *output,
                        GArray                       *items,
                        q02_arguments_output_filter_t filter) {

    for (size_t i = 0; i < items->len; ++i) {
        q02_output_item_t *item = &g_array_index(items, q02_output_item_t, i);

        char date_string[DATE_SPRINTF_MIN_BUFFER_SIZE];
        date_sprintf(date_string, date_and_time_get_date(item->date));

        char flight_id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
        char reservation_id_str[RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE];

        query_writer_write_new_object(output);
        switch (item->type) {
            case Q02_OUTPUT_ITEM_FLIGHT:
                flight_id_sprintf(flight_id_str, item->id);
                query_writer_write_new_field(output, "id", "%s", flight_id_str);
                break;
            case Q02_OUTPUT_ITEM_RESERVATION:
                reservation_id_sprintf(reservation_id_str, item->id);
                query_writer_write_new_field(output, "id", "%s", reservation_id_str);
                break;
        }
        query_writer_write_new_field(output, "date", "%s", date_string);

        if (filter == Q02_ARGUMENTS_NO_ARGUMENT) {
            const char *type = (item->type == Q02_OUTPUT_ITEM_FLIGHT) ? "flight" : "reservation";
            query_writer_write_new_field(output, "type", "%s", type);
        }
    }
}

/**
 * @brief Executes a query of type 2.
 *
 * @param database   Database to get information from.
 * @param statistics Always `NULL`, as this query does not use statistic data.
 * @param instance   Query instance to be executed.
 * @param output     Where to output query results to.
 *
 * @retval 0 Always. This query does not fail.
 */
int __q02_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  query_writer_t   *output) {

    (void) statistics;
    const q02_argument_data_t *args = query_instance_get_argument_data(instance);

    const user_manager_t        *users        = database_get_users(database);
    const reservation_manager_t *reservations = database_get_reservations(database);
    const flight_manager_t      *flights      = database_get_flights(database);

    const user_t *user = user_manager_get_by_id(users, args->user_id);
    if (!user || user_get_account_status(user) == ACCOUNT_STATUS_INACTIVE)
        return 0;

    GArray *output_items = g_array_new(FALSE, FALSE, sizeof(q02_output_item_t));

    if (args->filter != Q02_ARGUMENTS_FLIGHTS) { /* Add reservations */
        const single_pool_id_linked_list_t *user_reservations =
            user_manager_get_reservations_by_id(users, args->user_id);

        while (user_reservations) {
            uint32_t reservation_id = single_pool_id_linked_list_get_value(user_reservations);

            date_and_time_t output_time;
            date_and_time_from_values(
                &output_time,
                reservation_get_begin_date(
                    reservation_manager_get_by_id(reservations, reservation_id)),
                0 /* 00:00:00 */);

            q02_output_item_t output_item = {.id   = reservation_id,
                                             .date = output_time,
                                             .type = Q02_OUTPUT_ITEM_RESERVATION};

            g_array_append_val(output_items, output_item);
            user_reservations = single_pool_id_linked_list_get_next(user_reservations);
        }
    }

    if (args->filter != Q02_ARGUMENTS_RESERVATIONS) { /* Add flights */
        const single_pool_id_linked_list_t *user_flights =
            user_manager_get_flights_by_id(users, args->user_id);

        while (user_flights) {
            uint32_t flight_id = single_pool_id_linked_list_get_value(user_flights);

            q02_output_item_t output_item = {.id   = flight_id,
                                             .date = flight_get_schedule_departure_date(
                                                 flight_manager_get_by_id(flights, flight_id)),
                                             .type = Q02_OUTPUT_ITEM_FLIGHT};

            g_array_append_val(output_items, output_item);
            user_flights = single_pool_id_linked_list_get_next(user_flights);
        }
    }

    g_array_sort(output_items, __q02_execute_sort_compare);
    __q02_print_output(output, output_items, args->filter);

    g_array_free(output_items, TRUE);
    return 0;
}

query_type_t *q02_create(void) {
    return query_type_create(__q02_parse_arguments,
                             __q02_free_query_instance_argument_data,
                             NULL,
                             NULL,
                             __q02_execute);
}
