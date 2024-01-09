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
 * @file  q06.c
 * @brief Implementation of methods in include/queries/q06.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @struct q06_parsed_arguments_t
 * @brief A structure to hold the parsed arguments for a query of type 6.
 *
 * @var q06_parsed_arguments_t::year
 *     @brief The year to consider.
 * @var q06_parsed_arguments_t::n
 *    @brief The number of airports to display.
 */
typedef struct {
    uint16_t year;
    size_t   n;
} q06_parsed_arguments_t;

/**
 * @brief Parses the arguments for a query of type 6.
 * @details Asserts that there are exactly two arguments, a year and a number of airports to
 *          display.
 *
 * @param argv List of query arguments.
 * @param argc Number of query arguments.
 *
 * @return A pointer to a ::q06_parsed_arguments_t, or `NULL` on failure.
 */
void *__q06_parse_arguments(char *const *argv, size_t argc) {
    if (argc != 2)
        return NULL;

    q06_parsed_arguments_t *args = malloc(sizeof(q06_parsed_arguments_t));
    if (!args)
        return NULL;

    /* Parse year */
    size_t length = strlen(argv[0]);
    if (length == 4) {
        uint64_t year;
        int      retval = int_utils_parse_positive(&year, argv[0]);

        if (retval) {
            free(args);
            return NULL; /* Invalid year format */
        }

        args->year = year;
    } else {
        free(args);
        return NULL; /* Invalid year format */
    }

    /* Parse number of flights */
    uint64_t n;
    int      retval = int_utils_parse_positive(&n, argv[1]);
    if (retval) {
        free(args);
        return NULL; /* Invalid N format */
    }
    args->n = (size_t) n;

    return args;
}

void *__q06_clone_arguments(const void *args_data) {
    const q06_parsed_arguments_t *args  = args_data;
    q06_parsed_arguments_t       *clone = malloc(sizeof(q06_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q06_parsed_arguments_t));
    return clone;
}

/**
 * @brief   Adds a number of passengers to an airport in a `airport code -> passengers` hash table.
 * @details Auxiliary function for ::__q06_generate_statistics_foreach_flight, itself an auxiliary
 *          method for ::__q06_generate_statistics.
 *
 * @param airport_count  Hash table that associates `airport_code_t`s with numbers of passengers.
 * @param airport        Airport to add passengers to.
 * @param num_passengers Number of passengers to be added to @p airport in @p airport_count.
 */
void __q06_generate_statistics_add_passengers(GHashTable    *airport_count,
                                              airport_code_t airport,
                                              uint16_t       num_passengers) {
    uint64_t origin_count =
        GPOINTER_TO_UINT(g_hash_table_lookup(airport_count, GUINT_TO_POINTER(airport)));
    g_hash_table_insert(airport_count,
                        GUINT_TO_POINTER(airport),
                        GUINT_TO_POINTER(num_passengers + origin_count));
}

/**
 * @brief Method called for each flight, to generate statistical data.
 * @details An auxiliary method for ::__q06_generate_statistics.
 *
 * @param user_data A `GHashTable` that associates a year with another `GHashTable`, that itself
 *                  associates airport codes with integers numbers of passengers.
 * @param flight    The flight to consider.
 *
 * @retval 0 Always successful
 */
int __q06_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    GHashTable *years_airport_count = (GHashTable *) user_data;

    uint16_t year =
        date_get_year(date_and_time_get_date(flight_get_schedule_departure_date(flight)));

    /* Get the airport code -> passenger count mapping */
    GHashTable *airport_count = g_hash_table_lookup(years_airport_count, GUINT_TO_POINTER(year));
    if (!airport_count)
        return 0; /* Year not asked for in queries */

    airport_code_t origin         = flight_get_origin(flight);
    airport_code_t destination    = flight_get_destination(flight);
    uint16_t       num_passengers = flight_get_number_of_passengers(flight);

    __q06_generate_statistics_add_passengers(airport_count, origin, num_passengers);
    __q06_generate_statistics_add_passengers(airport_count, destination, num_passengers);

    return 0;
}

/**
 * @struct q06_array_item_t
 * @brief A structure to hold an item in the array of airport passanger counts.
 *
 * @var q06_array_item_t::airport
 *     @brief An airport.
 * @var q06_array_item_t::count
 *     @brief The number of passangers.
 */
typedef struct {
    airport_code_t airport;
    uint64_t       count;
} q06_array_item_t;

/**
 * @brief   Method called for each airport-passenger count pair in a `GHashTable`, that adds it to a
 *          `GArray`.
 * @details An auxiliary method for ::__q06_execute.
 *
 * @param key       The key in the `GHashTable`, an `airport_code_t`.
 * @param value     The value in the `GHashTable`, an integer (as a pointer).
 * @param user_data The `GArray` of airport + passanger count tuples (::q06_array_item_t).
 */
void __q06_foreach_airport_count(gpointer key, gpointer value, gpointer user_data) {
    q06_array_item_t item = {.airport = (airport_code_t) (size_t) key,
                             .count   = GPOINTER_TO_UINT(value)};
    g_array_append_val((GArray *) user_data, item);
}

/**
 * @brief   Comparison function for ::q06_array_item_t.
 * @details Used for array sorting in ::__q06_generate_statistics_foreach_year.
 */
gint __q06_sort_airports_by_count(gconstpointer a, gconstpointer b) {
    const q06_array_item_t *item_a = a;
    const q06_array_item_t *item_b = b;

    if (item_b->count == item_a->count) {
        char a_airport_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        char b_airport_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(a_airport_str, item_a->airport);
        airport_code_sprintf(b_airport_str, item_b->airport);

        return strcmp(a_airport_str, b_airport_str);
    } else {
        return item_b->count - item_a->count;
    }
}

/**
 * @brief   Converts a `GHashTable` to an ordered `GArray` of key-value tuples (::q06_array_item_t).
 * @details Called for every year.
 *
 * @param key_year            Year being processed (as a pointer).
 * @param value_airport_count `GHashTable` (`airport -> passenger count`) associated to @p key_year.
 * @param user_data           `GHashTable` (`year -> tuple array`) to write the output array to.
 */
void __q06_generate_statistics_foreach_year(gpointer key_year,
                                            gpointer value_airport_count,
                                            gpointer user_data) {

    int16_t     year          = GPOINTER_TO_UINT(key_year);
    GHashTable *airport_count = (GHashTable *) value_airport_count;

    GArray *airport_count_array =
        g_array_sized_new(FALSE, FALSE, sizeof(q06_array_item_t), g_hash_table_size(airport_count));

    g_hash_table_foreach(airport_count, __q06_foreach_airport_count, airport_count_array);
    g_array_sort(airport_count_array, __q06_sort_airports_by_count);

    g_hash_table_insert((GHashTable *) user_data, GUINT_TO_POINTER(year), airport_count_array);
}

/**
 * @brief Generates statistical data for queries of type 6.
 * @details Generates a `GHashTable` that associates years with sorted `GArray`s of
 *          ::q06_array_item_t (airport + passenger count tuples).
 *
 * @param database   Database to get data from.
 * @param instances  Array of query instances to be executed.
 * @param n          Number of query instances to be executed.
 *
 * @return A `GHashTable` that associates years with sorted `GArray`s of ::q06_array_item_t
 *         (airport + passenger count tuples).
 */
void *__q06_generate_statistics(const database_t              *database,
                                const query_instance_t *const *instances,
                                size_t                         n) {
    GHashTable *years_airport_count = g_hash_table_new_full(g_direct_hash,
                                                            g_direct_equal,
                                                            NULL,
                                                            (GDestroyNotify) g_hash_table_unref);

    for (size_t i = 0; i < n; ++i) {
        int16_t year =
            ((const q06_parsed_arguments_t *) query_instance_get_argument_data(instances[i]))->year;

        GHashTable *airport_count = g_hash_table_new(g_direct_hash, g_direct_equal);
        g_hash_table_insert(years_airport_count, GUINT_TO_POINTER(year), airport_count);
    }

    flight_manager_iter(database_get_flights(database),
                        __q06_generate_statistics_foreach_flight,
                        years_airport_count);

    /* Create a sorted array for each year (instead of a hash table) */
    GHashTable *years_airport_count_array =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) g_array_unref);

    g_hash_table_foreach(years_airport_count,
                         __q06_generate_statistics_foreach_year,
                         years_airport_count_array);

    g_hash_table_unref(years_airport_count);
    return years_airport_count_array;
}

/**
 * @brief   Executes a query of type 6.
 * @details Prints the top N airports with the most passangers in a given year.
 *
 * @param database   Database to get data from.
 * @param statistics Statistics generated by ::__q06_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     File to write output to.
 *
 * @retval 0 Always successful
 */
int __q06_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q06_parsed_arguments_t *args = query_instance_get_argument_data(instance);

    GHashTable *years_airport_count_array = (GHashTable *) statistics;
    GArray     *airport_count =
        (GArray *) g_hash_table_lookup(years_airport_count_array, GUINT_TO_POINTER(args->year));
    if (!airport_count) {
        fprintf(stderr, "Bad statistical data in query 6! This should not happen!\n");
        return 1; /* Only if statistics are bad, which shouldn't happen */
    }

    for (size_t i = 0; i < min(args->n, airport_count->len); ++i) {
        const q06_array_item_t *item = &g_array_index(airport_count, q06_array_item_t, i);

        char airport_code_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(airport_code_str, item->airport);

        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "name", "%s", airport_code_str);
        query_writer_write_new_field(output, "passengers", "%" PRIu64, item->count);
    }

    return 0;
}

query_type_t *q06_create(void) {
    return query_type_create(__q06_parse_arguments,
                             __q06_clone_arguments,
                             free,
                             __q06_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q06_execute);
}
