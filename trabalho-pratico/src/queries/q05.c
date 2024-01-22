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

#include <stdio.h>

#include "queries/q05.h"
#include "queries/query_instance.h"
#include "utils/glib/GConstKeyHashTable.h"
#include "utils/glib/GConstPtrArray.h"

/**
 * @struct q05_parsed_arguments_t
 * @brief  Parsed arguments of a query of type 5.
 *
 * @var q05_parsed_arguments_t::airport_code
 *     @brief Code of the origin airport.
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
 * @brief   Parses the arguments of a query of type 5.
 * @details Asserts that there's three arguments, an airport code, and two dates with times.
 *
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 *
 * @return `NULL` on parsing or allocation failure, a pointer to a ::q05_parsed_arguments_t
 *         otherwise.
 */
void *__q05_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 3)
        return NULL;

    q05_parsed_arguments_t *const parsed_arguments = malloc(sizeof(q05_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    const int airport_retval = airport_code_from_string(&parsed_arguments->airport_code, argv[0]);
    const int begin_date_retval = date_and_time_from_string(&parsed_arguments->begin_date, argv[1]);
    const int end_date_retval   = date_and_time_from_string(&parsed_arguments->end_date, argv[2]);

    if (airport_retval || begin_date_retval || end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }

    return parsed_arguments;
}

/**
 * @brief  Creates a deep clone of the value returned by ::__q05_parse_arguments.
 * @param  args_data Non-`NULL` value returned by ::__q05_parse_arguments (a pointer to a
 *                   ::q05_parsed_arguments_t).
 * @return A deep copy of @p args_data.
 */
void *__q05_clone_arguments(const void *args_data) {
    const q05_parsed_arguments_t *const args  = args_data;
    q05_parsed_arguments_t *const       clone = malloc(sizeof(q05_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q05_parsed_arguments_t));
    return clone;
}

/**
 * @struct q05_foreach_airport_data_t
 * @brief  Data needed while iterating over flights.
 *
 * @var q05_foreach_airport_data_t::nfilters
 *     @brief Number of elements in ::q05_foreach_airport_data_t::filters.
 * @var q05_foreach_airport_data_t::filters
 *     @brief Array of pointers to ::q05_parsed_arguments_t. Information about which flights and
 *            date ranges to keep.
 * @var q05_foreach_airport_data_t::origin_flights
 *     @brief Associations between ::q05_parsed_arguments_t and arrays (::GConstPtrArray) of
 *            pointers to flights (::flight_t).
 */
typedef struct {
    size_t                                     nfilters;
    const q05_parsed_arguments_t *const *const filters;
    GConstKeyHashTable *const                  origin_flights;
} q05_foreach_airport_data_t;

/**
 * @brief   A method called for each flight, to consider it in the list of flights with an origin
 *          on a given airport, depending on range of times.
 * @details An auxiliary method for ::__q05_generate_statistics.
 *
 * @param user_data A pointer to a ::q05_foreach_airport_data_t.
 * @param flight    Flight being processed.
 *
 * @retval 0 Always successful.
 */
int __q05_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    q05_foreach_airport_data_t *const foreach_data = user_data;

    airport_code_t  airport                 = flight_get_origin(flight);
    date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);

    /* Add flight to all query answers whose filter matches */
    for (size_t i = 0; i < foreach_data->nfilters; i++) {
        const q05_parsed_arguments_t *const args = foreach_data->filters[i];

        /* Check if the flight meets the filters in the arguments */
        if (airport == args->airport_code &&
            date_and_time_diff(args->begin_date, schedule_departure_date) <= 0 &&
            date_and_time_diff(args->end_date, schedule_departure_date) >= 0) {

            /* Add flight to answer */
            GConstPtrArray *const flights =
                g_const_key_hash_table_lookup(foreach_data->origin_flights, args);
            g_const_ptr_array_add(flights, flight);
        }
    }

    return 0;
}

/**
 * @brief   A comparison function for sorting a ::GConstPtrArray of flights.
 * @details Auxiliary method for ::__q05_generate_statistics_sort_each_array, itself an
 *          auxiliary method for ::__q05_generate_statistics.
 */
gint __q05_flights_date_compare_func(gconstpointer a, gconstpointer b) {
    const flight_t *const flight_a = *((const flight_t *const *) a);
    const flight_t *const flight_b = *((const flight_t *const *) b);

    const int64_t crit1 = date_and_time_diff(flight_get_schedule_departure_date(flight_b),
                                             flight_get_schedule_departure_date(flight_a));
    if (crit1)
        return crit1;

    const int32_t crit2 = flight_get_id(flight_a) - flight_get_id(flight_b);
    return crit2;
}

/**
 * @brief Sorts each array of flights in the statistical's data hash table.
 *
 * @param query Pointer to a ::q05_foreach_airport_data_t.
 * @param list  Array of reservations to be sorted (::GConstPtrArray).
 * @param data  External data (not used).
 */
void __q05_generate_statistics_sort_each_array(gconstpointer query, gpointer list, gpointer data) {
    (void) query;
    (void) data;
    g_const_ptr_array_sort(list, __q05_flights_date_compare_func);
}

/**
 * @brief Generates statistical data for queries of type 5.
 *
 * @param database  Database, to iterate through flights.
 * @param n         Number of query instances that need to be executed.
 * @param instances Query instances that need to be executed.
 *
 * @return A ::GConstKeyHashTable associating a ::q05_foreach_airport_data_t for each query to a
 *         ::GConstPtrArray of pointers to flights (::flight_t). `NULL` may be returned on
 *         allocation failure.
 */
void *__q05_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {
    const q05_parsed_arguments_t **const filters = malloc(n * sizeof(q05_parsed_arguments_t *));
    if (!filters)
        return NULL;

    GConstKeyHashTable *const origin_flights =
        g_const_key_hash_table_new_full(g_direct_hash,
                                        g_direct_equal,
                                        (GDestroyNotify) g_ptr_array_unref);

    for (size_t i = 0; i < n; ++i) {
        const q05_parsed_arguments_t *const argument_data =
            query_instance_get_argument_data(instances[i]);

        g_const_key_hash_table_insert(origin_flights, argument_data, g_const_ptr_array_new());
        filters[i] = argument_data;
    }

    q05_foreach_airport_data_t callback_data = {.nfilters       = n,
                                                .filters        = filters,
                                                .origin_flights = origin_flights};
    flight_manager_iter(database_get_flights(database),
                        __q05_generate_statistics_foreach_flight,
                        &callback_data);

    g_const_key_hash_table_foreach(origin_flights, __q05_generate_statistics_sort_each_array, NULL);
    free(filters);
    return origin_flights;
}

/**
 * @brief Method called to execute a query of type 5.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q05_generate_statistics).
 * @param statistics Statistical data generated by ::__q05_generate_statistics (a
 *                   ::GConstKeyHashTable that associates query arguments (::q05_parsed_arguments_t)
 *                   to arrays (::GConstPtrArray) of flights (::flight_id_t)).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success.
 * @retval 1 Fatal failure (should, in principle, be unreachable).
 */
int __q05_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const GConstKeyHashTable *const     origin_flights = statistics;
    const q05_parsed_arguments_t *const arguments      = query_instance_get_argument_data(instance);

    const GConstPtrArray *const flights =
        g_const_key_hash_table_const_lookup(origin_flights, arguments);
    if (!flights) {
        fprintf(stderr, "Bad statistical data in query 5! This should not happen!\n");
        return 1; /* Only happens on bad statistics, which itself shouldn't happen. */
    }

    const size_t flights_len = g_const_ptr_array_get_length(flights);
    for (size_t i = 0; i < flights_len; i++) {
        const flight_t *const flight = g_const_ptr_array_index(flights, i);

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
    return query_type_create(5,
                             __q05_parse_arguments,
                             __q05_clone_arguments,
                             free,
                             __q05_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q05_execute);
}
