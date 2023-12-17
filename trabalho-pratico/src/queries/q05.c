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
#include "types/airport_code.h"
#include "utils/date_and_time.h"

/**
 * @struct q05_foreach_airport_data_t
 * @brief  Data needed while iterating over flights.
 *
 * @var q05_foreach_airport_data_t::airports_data
 *     @brief Stores data for all the airports that were queried.
 * @var q05_foreach_airport_data_t::origin_flights
 *     @brief Stores arrays of flights originated from a specific queried airport.
 */
typedef struct {
    GArray     *airports_data;
    GHashTable *origin_flights;
} q05_foreach_airport_data_t;

/**
 * @struct q05_limit_dates_t
 * @brief  Stores the time frame of a queried airport.
 *
 * @var q05_limit_dates_t::begin_date
 *     @brief The time frame's starting date.
 * @var q_05_limit_dates_t::end_date
 *     @brief The time frame's ending date.
 */
typedef struct {
    date_and_time_t begin_date;
    date_and_time_t end_date;
} q05_limit_dates_t;

/**
 * @struct q05_parsed_arguments_t
 * @brief  Data needed for the execution of a query of type 5.
 *
 * @var q05_parsed_arguments_t::airport_code
 *     @brief Parsed origin airport code.
 * @var q05_parsed_arguments_t::dates
 *     @brief A pointer to an `q05_limit_dates_t`, which stores the parsed time frame.
 */
typedef struct {
    airport_code_t     airport_code;
    q05_limit_dates_t *dates;
} q05_parsed_arguments_t;

/**
 * @struct q05_airport_data
 * @brief  Stores an origin airport and all its queried time frames, to use when iterating over the
 *         flights.
 *
 * @var q05_airport_data::airport_code
 *     @brief Origin airport code.
 * @var q05_airport_data::dates
 *     @brief An array of all the time frames queried for a specific origin airport.
 */
typedef struct {
    airport_code_t airport_code;
    GArray        *dates;
} q05_airport_data_t;

/**
 * @brief   Parses arguments for the fifth query.
 * @details Asserts that there's three arguments, an airport code, and two dates with time.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` on failure, a pointer to a `q05_airport_data_t` otherwise.
 */
void *__q05_parse_arguments(char **argv, size_t argc) {
    if (argc != 3)
        return NULL;

    q05_parsed_arguments_t *parsed_arguments = malloc(sizeof(q05_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    parsed_arguments->dates = malloc(sizeof(q05_limit_dates_t));
    if (!parsed_arguments->dates) {
        free(parsed_arguments);
        return NULL;
    }

    airport_code_t airport_code;
    int            airport_retval = airport_code_from_string(&airport_code, argv[0]);
    if (airport_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->airport_code = airport_code;

    date_and_time_t begin_date;
    int             begin_date_retval = date_and_time_from_string(&begin_date, argv[1]);
    if (begin_date_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->dates->begin_date = begin_date;

    date_and_time_t end_date;
    int             end_date_retval = date_and_time_from_string(&end_date, argv[2]);
    if (end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->dates->end_date = end_date;

    return parsed_arguments;
}

/**
 * @brief   A comparison function for sorting a `GArray` of `uint32_t`s.
 * @details Auxiliary method for ::__q05_generate_statistics_foreach_airport.
 */
gint __q05_foreach_airport_uint32_compare_func(gconstpointer a, gconstpointer b) {
    q05_airport_data_t *parsed_arguments = *(q05_airport_data_t **) a;
    airport_code_t     *airport_code     = (airport_code_t *) b;

    return ((int32_t) parsed_arguments->airport_code - (int32_t) *airport_code);
}

/**
 * @brief   A method called for each flight, to consider it in the list of flights with an origin
 *          on a given airport, dependent on a time frame.
 * @details An auxiliary method for ::__q05_generate_statistics.
 *
 * @param user_data   A pointer to a ::q05_foreach_airport_data_t.
 * @param reservation Flight being processed.
 *
 * @retval 0 Always successful.
 */
int __q05_generate_statistics_foreach_airport(void *user_data, const flight_t *flight) {
    q05_foreach_airport_data_t *foreach_data = (q05_foreach_airport_data_t *) user_data;

    airport_code_t  airport                 = flight_get_origin(flight);
    date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);

    guint   airport_index;
    GArray *airports_data = foreach_data->airports_data;

    if (!g_array_binary_search(airports_data,
                               &airport,
                               __q05_foreach_airport_uint32_compare_func,
                               &airport_index))
        return 0; /* airport not found */

    q05_airport_data_t *airport_data =
        g_array_index(airports_data, q05_airport_data_t *, airport_index);

    for (size_t i = 0; i < airport_data->dates->len; i++) {
        q05_limit_dates_t *single_airport_dates =
            g_array_index(airport_data->dates, q05_limit_dates_t *, i);

        date_and_time_t end_date   = single_airport_dates->end_date;
        date_and_time_t begin_date = single_airport_dates->begin_date;

        /* Checks if the flight scheduled departure date is in between the given dates */
        if (date_and_time_diff(begin_date, schedule_departure_date) <= 0 &&
            date_and_time_diff(end_date, schedule_departure_date) >= 0) {

            GArray *flights = g_hash_table_lookup(foreach_data->origin_flights,
                                                  GUINT_TO_POINTER(airport + single_airport_dates));

            if (!flights) {
                flights = g_array_new(FALSE, FALSE, sizeof(flight_t *));

                g_hash_table_insert(foreach_data->origin_flights,
                                    GUINT_TO_POINTER(airport + single_airport_dates),
                                    flights);
            }

            g_array_append_val(flights, flight);
        }
    }

    return 0;
}

/**
  * @brief   A comparison function for sorting a `GArray` of flights by date.
  * @details Auxiliary method for ::__q05_generate_statistics_sort_each_array, itself an
  *          auxiliary method for ::__q05_generate_statistics.
  */
gint __q05_flights_date_compare_func(gconstpointer a, gconstpointer b) {
    const flight_t *flight_a = *((flight_t **) a);
    const flight_t *flight_b = *((flight_t **) b);

    int64_t diff = date_and_time_diff(flight_get_schedule_departure_date(flight_b),
                                      flight_get_schedule_departure_date(flight_a));

    if (diff == 0)
        return flight_get_id(flight_a) - flight_get_id(flight_b);
    else
        return diff;
}

/**
 * @brief Sorts each array of flights. Iteration function for a hash table.
 *
 * @param key       Key used for referencing items in the hash table, not used.
 * @param value     `GArray` of pointers to `flight_t`s to be sorted.
 * @param user_data Not used (`NULL` is provided).
 */
void __q05_generate_statistics_sort_each_array(gpointer key, gpointer value, gpointer user_data) {
    (void) key;
    (void) user_data;

    g_array_sort((GArray *) value, __q05_flights_date_compare_func);
}

/**
 * @brief   A comparison function for sorting a `GArray` of `uint64_t`s.
 * @details Auxiliary method for ::__q05_generate_statistics.
 */
gint __q05_generate_statistics_uint64_compare_func(gconstpointer a, gconstpointer b) {
    q05_airport_data_t *argument_data_a = *(q05_airport_data_t **) a;
    q05_airport_data_t *argument_data_b = *(q05_airport_data_t **) b;

    return ((int64_t) argument_data_a->airport_code - (int64_t) argument_data_b->airport_code);
}

/**
 * @brief Creates and adds a new element of type `q05_airport_data_t` to a `GArray`.
 *
 * @param airports_data `GArray` to have the new element.
 * @param argument_data Element to be added to `airports_data`.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __q05_add_new_airport_data(GArray *airports_data, const q05_parsed_arguments_t *argument_data) {

    GArray *dates = g_array_new(FALSE, FALSE, sizeof(q05_limit_dates_t *));
    g_array_append_val(dates, argument_data->dates);

    q05_airport_data_t *new_airport = malloc(sizeof(q05_airport_data_t));
    if (!new_airport)
        return 1; /* Allocation failure */

    new_airport->airport_code = argument_data->airport_code;
    new_airport->dates        = dates;

    g_array_append_val(airports_data, new_airport);
    return 0;
}

/**
 * @brief A function to spot and handle possible duplications in a `GArray`.
 *
 * @param airports_data `GArray` to be searched.
 * @param argument_data Argument with an airport code to be compared with each element in
 *                      `airports_data`, to check if the airport already exists.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __q05_generate_statistics_handle_duplicates(GArray                       *airports_data,
                                                const q05_parsed_arguments_t *argument_data) {

    for (size_t i = 0; i < airports_data->len; i++) {
        q05_airport_data_t *airport_data = g_array_index(airports_data, q05_airport_data_t *, i);

        if (airport_data->airport_code == argument_data->airport_code) {
            g_array_append_val(airport_data->dates, argument_data->dates);
            return 0; /* The airport was found in the array, new time frame added */
        }
    }

    /* airport wasn't found, and should be treated as a new element */
    if (__q05_add_new_airport_data(airports_data, argument_data)) {
        return 1; /* Allocation failure */
    }

    return 0;
}

/**
 * @brief   Frees an `GArray`.
 * @details Auxiliary method to ::__q05_generate_statistics.
 *
 * @param airports_data `GArray` to be freed.
 */
void __q05_generate_statistics_free_airports_data(GArray *airports_data) {

    for (size_t i = 0; i < airports_data->len; i++) {
        q05_airport_data_t *airport_data = g_array_index(airports_data, q05_airport_data_t *, i);
        g_array_free(airport_data->dates, TRUE);
        free(airport_data);
    }
    g_array_free(airports_data, TRUE);
}

/**
 * @brief Generates statistical data for queries of type 5.
 *
 * @param database   Database, to iterate through flights.
 * @param instances  Instances of the query.
 * @param n          Number of instances.
 *
 * @return A `GHashTable` associating airport codes in addition to unique pointers (their own
 *         pointers to a `q05_limit_dates_t`), to `GArray`s of `flight_t`s.
 */
void *__q05_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) database;

    /*
     * Generate an array of all airport codes that will be asked for in queries, and their necessary
     * data.
     */
    GArray *airports_data = g_array_new(FALSE, FALSE, sizeof(q05_airport_data_t *));
    for (size_t i = 0; i < n; ++i) {
        const q05_parsed_arguments_t *argument_data =
            (q05_parsed_arguments_t *) query_instance_get_argument_data(instances);

        if (__q05_generate_statistics_handle_duplicates(airports_data, argument_data)) {
            __q05_generate_statistics_free_airports_data(airports_data);
            return NULL; /* Allocation failure */
        }

        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    g_array_sort(airports_data, __q05_generate_statistics_uint64_compare_func);

    GHashTable *origin_flights = g_hash_table_new(g_direct_hash, g_direct_equal);

    q05_foreach_airport_data_t callback_data = {.airports_data  = airports_data,
                                                .origin_flights = origin_flights};

    if (flight_manager_iter(database_get_flights(database),
                            __q05_generate_statistics_foreach_airport,
                            &callback_data)) { /* Allocation failure */

        __q05_generate_statistics_free_airports_data(airports_data);
        g_hash_table_destroy(origin_flights);
        return NULL;
    }
    __q05_generate_statistics_free_airports_data(airports_data);

    g_hash_table_foreach(origin_flights, __q05_generate_statistics_sort_each_array, NULL);
    return origin_flights;
}

/**
 * @brief Frees an array of `flight_t`s. Iteration function for a hash table.
 *
 * @param key       Key used for referencing items in the hash table, not used.
 * @param value     `GArray` of pointers to `flight_t`s to be freed.
 * @param user_data Not used (`NULL` is provided).
 */
void __q05_free_hashtable_values(gpointer key, gpointer value, gpointer user_data) {
    (void) key;
    (void) user_data;

    g_array_unref((GArray *) value);
}

/**
 * @brief Frees statistical data generated by ::__q05_generate_statistics.
 * @param statistics Data generated by ::__q05_generate_statistics.
 */
void __q05_free_statistics(void *statistics) {
    GHashTable *origin_flights = (GHashTable *) statistics;

    g_hash_table_foreach(origin_flights, __q05_free_hashtable_values, NULL);
    g_hash_table_destroy(origin_flights);
}

/**
 * @brief Frees the value returned by ::__q05_parse_arguments.
 * @param argument_data Value returned by ::__q05_parse_arguments.
 */
void __q05_free_query_instance_argument_data(void *argument_data) {
    q05_parsed_arguments_t *parsed_arguments = (q05_parsed_arguments_t *) argument_data;

    free(parsed_arguments->dates);
    free(parsed_arguments);
}

/**
 * @brief Method called to execute a query of type 5.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q05_generate_statistics).
 * @param statistics Statistical data generated by ::__q05_generate_statistics (a hashtable that
 *                   associates airport codes in addition to unique pointers (their own pointers to
 *                   a `q05_time_limits_t`), to `GArray`s of flights).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success
 * @retval 1 Failure (Happens when there is no flight that meets an argument's requirements).
 */
int __q05_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    GHashTable             *origin_flights = (GHashTable *) statistics;
    q05_parsed_arguments_t *arguments =
        (q05_parsed_arguments_t *) query_instance_get_argument_data(instance);

    GArray *flights =
        g_hash_table_lookup(origin_flights,
                            GUINT_TO_POINTER(arguments->airport_code + arguments->dates));
    if (!flights) {
        return 1; /* No flight complied with the time frame of the argument */
    }
    size_t flights_len = flights->len;

    if (query_instance_get_formatted(instance)) {
        for (size_t i = 0; i < flights_len; i++) {
            flight_t *flight = g_array_index(flights, flight_t *, i);

            char scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
            date_and_time_sprintf(scheduled_departure_str,
                                  flight_get_schedule_departure_date(flight));

            char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
            airport_code_sprintf(destination_airport, flight_get_destination(flight));

            fprintf(output,
                    "--- %ld ---\nid: %010" PRIu64 "\nschedule_departure_date: %s\n"
                    "destination: %s\nairline: %s\nplane_model: %s\n",
                    i + 1,
                    flight_get_id(flight),
                    scheduled_departure_str,
                    destination_airport,
                    flight_get_const_airline(flight),
                    flight_get_const_plane_model(flight));

            if (i != flights_len - 1)
                fputc('\n', output);
        }
    } else {
        for (size_t i = 0; i < flights_len; i++) {
            flight_t *flight = g_array_index(flights, flight_t *, i);

            char scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
            date_and_time_sprintf(scheduled_departure_str,
                                  flight_get_schedule_departure_date(flight));

            char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
            airport_code_sprintf(destination_airport, flight_get_destination(flight));

            fprintf(output,
                    "%010" PRIu64 ";%s;%s;%s;%s\n",
                    flight_get_id(flight),
                    scheduled_departure_str,
                    destination_airport,
                    flight_get_const_airline(flight),
                    flight_get_const_plane_model(flight));
        }
    }

    return 0;
}

query_type_t *q05_create(void) {
    return query_type_create(__q05_parse_arguments,
                             __q05_free_query_instance_argument_data,
                             __q05_generate_statistics,
                             __q05_free_statistics,
                             __q05_execute);
}
