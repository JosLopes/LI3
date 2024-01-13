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
 * @file  q05.c
 * @brief Implementation of methods in include/queries/q05.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/q05.h"
#include "queries/query_instance.h"

/**
 * @struct q05_parsed_arguments_t
 * @brief  Data needed for the execution of a query of type 5.
 *
 * @var q05_parsed_arguments_t::airport_code
 *     @brief Parsed origin airport code.
 * @var q05_parsed_arguments_t::begin_date
 *     @brief Beginning date for date range filter.
 * @var q05_parsed_arguments_t::end_date
 *     @brief End date for date range filter.
 */
typedef struct {
    airport_code_t  airport_code;
    date_and_time_t begin_date;
    date_and_time_t end_date;
} q05_parsed_arguments_t;

/**
 * @brief   Parses arguments for the fifth query.
 * @details Asserts that there's three arguments, an airport code, and two dates with time.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` on failure, a pointer to a `q05_airport_data_t` otherwise.
 */
void *__q05_parse_arguments(char *const *argv, size_t argc) {
    if (argc != 3)
        return NULL;

    q05_parsed_arguments_t *parsed_arguments = malloc(sizeof(q05_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    int airport_retval    = airport_code_from_string(&parsed_arguments->airport_code, argv[0]);
    int begin_date_retval = date_and_time_from_string(&parsed_arguments->begin_date, argv[1]);
    int end_date_retval   = date_and_time_from_string(&parsed_arguments->end_date, argv[2]);

    if (airport_retval || begin_date_retval || end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }

    return parsed_arguments;
}

void *__q05_clone_arguments(const void *args_data) {
    const q05_parsed_arguments_t *args  = args_data;
    q05_parsed_arguments_t       *clone = malloc(sizeof(q05_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q05_parsed_arguments_t));
    return clone;
}

/**
 * @struct q05_foreach_airport_data_t
 * @brief  Data needed while iterating over flights.
 *
 * @var q05_foreach_airport_data_t::filter_data
 *     @brief Array of pointers to ::q05_parsed_arguments_t. Information about which flights to
 *            keep.
 * @var q05_foreach_airport_data_t::origin_flights
 *     @brief Associations between ::q05_parsed_arguments_t and `GPtrArray`s of pointers to
 *            ::flight_t.
 */
typedef struct {
    GPtrArray  *filter_data;
    GHashTable *origin_flights;
} q05_foreach_airport_data_t;

/**
 * @brief   A method called for each flight, to consider it in the list of flights with an origin
 *          on a given airport, depending on a time frame.
 * @details An auxiliary method for ::__q05_generate_statistics.
 *
 * @param user_data A pointer to a ::q05_foreach_airport_data_t.
 * @param flight    Flight being processed.
 *
 * @retval 0 Always successful.
 */
int __q05_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    q05_foreach_airport_data_t *foreach_data = (q05_foreach_airport_data_t *) user_data;

    airport_code_t  airport                 = flight_get_origin(flight);
    date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);

    /* Add flight to all query answers whose filter matches */
    for (size_t i = 0; i < foreach_data->filter_data->len; i++) {
        const q05_parsed_arguments_t *args = g_ptr_array_index(foreach_data->filter_data, i);

        /* Check if the flight meets the filters in the arguments */
        if (airport == args->airport_code &&
            date_and_time_diff(args->begin_date, schedule_departure_date) <= 0 &&
            date_and_time_diff(args->end_date, schedule_departure_date) >= 0) {

            /* Add flight to answer (create answer array if needed) */
            GPtrArray *flights = g_hash_table_lookup(foreach_data->origin_flights, args);
            /* TODO - find a way to keep const */
            g_ptr_array_add(flights, (flight_t *) flight);
        }
    }

    return 0;
}

/**
 * @brief Generates statistical data for queries of type 5.
 *
 * @param database   Database, to iterate through flights.
 * @param instances  Instances of the query 5.
 * @param n          Number of query instances.
 *
 * @return A `GHashTable` associating a ::q05_foreach_airport_data_t for each query to a
 *         `GPtrArray` of ::flight_t `*`s.
 */
void *__q05_generate_statistics(const database_t              *database,
                                const query_instance_t *const *instances,
                                size_t                         n) {
    GPtrArray  *filter_data    = g_ptr_array_new();
    GHashTable *origin_flights = g_hash_table_new_full(g_direct_hash,
                                                       g_direct_equal,
                                                       NULL,
                                                       (GDestroyNotify) g_ptr_array_unref);

    for (size_t i = 0; i < n; ++i) {
        const q05_parsed_arguments_t *argument_data =
            query_instance_get_argument_data(instances[i]);

        /* TODO - find way of keeping const */
        g_hash_table_insert(origin_flights,
                            (q05_parsed_arguments_t *) argument_data,
                            g_ptr_array_new());
        g_ptr_array_add(filter_data, (q05_parsed_arguments_t *) argument_data);
    }

    q05_foreach_airport_data_t callback_data = {.filter_data    = filter_data,
                                                .origin_flights = origin_flights};

    flight_manager_iter(database_get_flights(database),
                        __q05_generate_statistics_foreach_flight,
                        &callback_data);

    g_ptr_array_unref(filter_data);
    return origin_flights;
}

/**
  * @brief   A comparison function for sorting a `GPtrArray` of flights by date.
  * @details Auxiliary method for ::__q05_generate_statistics_sort_each_array, itself an
  *          auxiliary method for ::__q05_generate_statistics.
  */
gint __q05_flights_date_compare_func(gconstpointer a, gconstpointer b) {
    const flight_t *flight_a = *((const flight_t *const *) a);
    const flight_t *flight_b = *((const flight_t *const *) b);

    int64_t crit1 = date_and_time_diff(flight_get_schedule_departure_date(flight_b),
                                       flight_get_schedule_departure_date(flight_a));
    if (crit1)
        return crit1;

    int32_t crit2 = flight_get_id(flight_a) - flight_get_id(flight_b);
    return crit2;
}

/**
 * @brief Method called to execute a query of type 5.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q05_generate_statistics).
 * @param statistics Statistical data generated by ::__q05_generate_statistics (a `GHashTable` that
 *                   associates ::q05_parsed_arguments_t to `GPtrArray`s of flights).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Always successful
 */
int __q05_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    GHashTable                   *origin_flights = (GHashTable *) statistics;
    const q05_parsed_arguments_t *arguments      = query_instance_get_argument_data(instance);

    GPtrArray *flights = g_hash_table_lookup(origin_flights, arguments);
    if (!flights)
        return 0;

    /* Sorting is only done once per query */
    g_ptr_array_sort(flights, __q05_flights_date_compare_func);
    for (size_t i = 0; i < flights->len; i++) {
        const flight_t *flight = g_ptr_array_index(flights, i);

        char scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
        date_and_time_sprintf(scheduled_departure_str, flight_get_schedule_departure_date(flight));

        char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(destination_airport, flight_get_destination(flight));

        char flight_id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
        flight_id_sprintf(flight_id_str, flight_get_id(flight));

        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "id", "%s", flight_id_str);
        query_writer_write_new_field(output,
                                     "schedule_departure_date",
                                     "%s",
                                     scheduled_departure_str);
        query_writer_write_new_field(output, "destination", "%s", destination_airport);
        query_writer_write_new_field(output, "airline", "%s", flight_get_const_airline(flight));
        query_writer_write_new_field(output,
                                     "plane_model",
                                     "%s",
                                     flight_get_const_plane_model(flight));
    }

    return 0;
}

query_type_t *q05_create(void) {
    return query_type_create(__q05_parse_arguments,
                             __q05_clone_arguments,
                             free,
                             __q05_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q05_execute);
}
