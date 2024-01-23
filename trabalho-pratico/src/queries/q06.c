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

#include <stdio.h>

#include "queries/q06.h"
#include "queries/query_instance.h"
#include "utils/glib/GConstKeyHashTable.h"
#include "utils/int_utils.h"

/**
 * @struct q06_parsed_arguments_t
 * @brief  Parsed arguments for a query of type 6.
 *
 * @var q06_parsed_arguments_t::n
 *    @brief The number of airports to display.
 * @var q06_parsed_arguments_t::year
 *     @brief The year to consider.
 */
typedef struct {
    size_t   n;
    uint16_t year;
} q06_parsed_arguments_t;

/**
 * @brief   Parses the arguments for a query of type 6.
 * @details Asserts that there are exactly two arguments, a year and a number of airports to
 *          display.
 *
 * @param argc Number of query arguments.
 * @param argv List of query arguments.
 *
 * @return A pointer to a ::q06_parsed_arguments_t, or `NULL` on parsing or allocation failure.
 */
void *__q06_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 2)
        return NULL;

    q06_parsed_arguments_t *const args = malloc(sizeof(q06_parsed_arguments_t));
    if (!args)
        return NULL;

    /* Parse year */
    const size_t length = strlen(argv[0]);
    if (length == 4) {
        uint64_t  year;
        const int retval = int_utils_parse_positive(&year, argv[0]);

        if (retval) {
            free(args);
            return NULL; /* Invalid year format */
        }

        args->year = year;
    } else {
        free(args);
        return NULL; /* Invalid year format */
    }

    /* Parse the number of flights */
    uint64_t  n;
    const int retval = int_utils_parse_positive(&n, argv[1]);
    if (retval) {
        free(args);
        return NULL; /* Invalid N format */
    }
    args->n = (size_t) n;

    return args;
}

/**
 * @brief  Creates a deep clone of the value returned by ::__q06_parse_arguments.
 * @param  args_data Non-`NULL` value returned by ::__q06_parse_arguments (a pointer to a
 *                   ::q06_parsed_arguments_t).
 * @return A deep copy of @p args_data.
 */
void *__q06_clone_arguments(const void *args_data) {
    const q06_parsed_arguments_t *const args  = args_data;
    q06_parsed_arguments_t *const       clone = malloc(sizeof(q06_parsed_arguments_t));
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
 * @param airport_count  `GHashTable` that associates airports (::airport_code_t) with numbers of
 *                       passengers.
 * @param airport        Airport to add passengers to.
 * @param num_passengers Number of passengers to be added to @p airport in @p airport_count.
 */
void __q06_generate_statistics_add_passengers(GHashTable    *airport_count,
                                              airport_code_t airport,
                                              uint16_t       num_passengers) {
    const uint64_t origin_count =
        GPOINTER_TO_UINT(g_hash_table_lookup(airport_count, GUINT_TO_POINTER(airport)));
    g_hash_table_insert(airport_count,
                        GUINT_TO_POINTER(airport),
                        GUINT_TO_POINTER(num_passengers + origin_count));
}

/**
 * @brief   Method called for each flight, to generate statistical data.
 * @details An auxiliary method for ::__q06_generate_statistics.
 *
 * @param user_data A `GHashTable` that associates a year with another `GHashTable`, that itself
 *                  associates airport codes with numbers of passengers (integers).
 * @param flight    The flight to consider.
 *
 * @retval 0 Always successful.
 */
int __q06_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    const uint16_t year =
        date_get_year(date_and_time_get_date(flight_get_schedule_departure_date(flight)));

    /* Get the airport code -> passenger count mapping */
    GHashTable *const airport_count = g_hash_table_lookup(user_data, GUINT_TO_POINTER(year));
    if (!airport_count)
        return 0; /* Year not asked for in queries */

    const airport_code_t origin         = flight_get_origin(flight);
    const airport_code_t destination    = flight_get_destination(flight);
    const uint16_t       num_passengers = flight_get_number_of_passengers(flight);

    __q06_generate_statistics_add_passengers(airport_count, origin, num_passengers);
    __q06_generate_statistics_add_passengers(airport_count, destination, num_passengers);
    return 0;
}

/**
 * @struct q06_array_item_t
 * @brief  A pair formed by an airport and its number of passengers.
 *
 * @var q06_array_item_t::airport
 *     @brief An airport.
 * @var q06_array_item_t::count
 *     @brief The number of passangers of q06_array_item_t::airport.
 */
typedef struct {
    airport_code_t airport;
    uint32_t       count;
} q06_array_item_t;

/**
 * @brief   Method called for each airport-passenger count pair in a `GHashTable`, which is added to
 *          a `GArray`.
 * @details An auxiliary method for ::__q06_generate_statistics_foreach_year.
 *
 * @param key       The key in the `GHashTable`, an ::airport_code_t.
 * @param value     The value in the `GHashTable`, an integer (as a pointer).
 * @param user_data The `GArray` of airport + passanger count tuples (::q06_array_item_t).
 */
void __q06_foreach_airport_count(gpointer key, gpointer value, gpointer user_data) {
    const q06_array_item_t item = {.airport = GPOINTER_TO_UINT(key),
                                   .count   = GPOINTER_TO_UINT(value)};
    g_array_append_val(user_data, item);
}

/**
 * @brief   Comparison function for ::q06_array_item_t.
 * @details Used for array sorting in ::__q06_generate_statistics_foreach_year.
 */
gint __q06_sort_airports_by_count(gconstpointer a, gconstpointer b) {
    const q06_array_item_t *const item_a = a;
    const q06_array_item_t *const item_b = b;

    const int64_t crit1 = item_b->count - item_a->count;
    if (crit1)
        return crit1;

    char a_airport_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    char b_airport_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(a_airport_str, item_a->airport);
    airport_code_sprintf(b_airport_str, item_b->airport);

    return strcmp(a_airport_str, b_airport_str);
}

/**
 * @brief   Converts a `GHashTable` (::airport_code_t -> passenger count) to an ordered `GArray` of
 *          key-value tuples (::q06_array_item_t).
 * @details The `GArray` will be inserted into a ::GConstKeyHashTable whose keys are years. This is
 *          an auxiliary method for ::__q06_generate_statistics.
 *
 * @param key_year            Year being processed (as a pointer).
 * @param value_airport_count `GHashTable` (::airport_code_t -> passenger count) associated to
 *                            @p key_year.
 * @param user_data           ::GConstKeyHashTable (year -> tuple array) to write the output array
 *                            to.
 */
void __q06_generate_statistics_foreach_year(gpointer key_year,
                                            gpointer value_airport_count,
                                            gpointer user_data) {

    GHashTable *const airport_count = value_airport_count;
    GArray *const     airport_count_array =
        g_array_sized_new(FALSE, FALSE, sizeof(q06_array_item_t), g_hash_table_size(airport_count));

    g_hash_table_foreach(airport_count, __q06_foreach_airport_count, airport_count_array);
    g_array_sort(airport_count_array, __q06_sort_airports_by_count);

    g_const_key_hash_table_insert(user_data, key_year, airport_count_array);
}

/**
 * @brief Generates statistical data for queries of type 6.
 *
 * @param database   Database, to iterate through flights.
 * @param n          Number of query instances to be executed.
 * @param instances  Array of query instances to be executed.
 *
 * @return A ::GConstKeyHashTable that associates years with sorted `GArray`s of ::q06_array_item_t
 *         (airport + passenger count tuples).
 */
void *__q06_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {

    GHashTable *const years_airport_count =
        g_hash_table_new_full(g_direct_hash,
                              g_direct_equal,
                              NULL,
                              (GDestroyNotify) g_hash_table_unref);
    for (size_t i = 0; i < n; ++i) {
        const int16_t year =
            ((const q06_parsed_arguments_t *) query_instance_get_argument_data(instances[i]))->year;

        GHashTable *const airport_count = g_hash_table_new(g_direct_hash, g_direct_equal);
        g_hash_table_insert(years_airport_count, GUINT_TO_POINTER(year), airport_count);
    }

    flight_manager_iter(database_get_flights(database),
                        __q06_generate_statistics_foreach_flight,
                        years_airport_count);

    /* Create a sorted array for each year (instead of a hash table) */
    GConstKeyHashTable *const years_airport_count_array =
        g_const_key_hash_table_new_full(g_direct_hash,
                                        g_direct_equal,
                                        (GDestroyNotify) g_array_unref);
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
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q06_generate_statistics).
 * @param statistics Statistics generated by ::__q06_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success.
 * @retval 1 Fatal failure (should, in principle, be unreachable).
 */
int __q06_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q06_parsed_arguments_t *const args = query_instance_get_argument_data(instance);

    const GConstKeyHashTable *const years_airport_count_array = statistics;
    const GArray *const             airport_count =
        g_const_key_hash_table_const_lookup(years_airport_count_array,
                                            GUINT_TO_POINTER(args->year));

    if (!airport_count) {
        fprintf(stderr, "Bad statistical data in query 6! This should not happen!\n");
        return 1; /* Only if statistics are bad, which shouldn't happen */
    }

    const size_t i_max = min(args->n, airport_count->len);
    for (size_t i = 0; i < i_max; ++i) {
        const q06_array_item_t *const item = &g_array_index(airport_count, q06_array_item_t, i);

        char airport_code_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(airport_code_str, item->airport);

        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "name", "%s", airport_code_str);
        query_writer_write_new_field(output, "passengers", "%" PRIu32, item->count);
    }

    return 0;
}

query_type_t *q06_create(void) {
    return query_type_create(6,
                             __q06_parse_arguments,
                             __q06_clone_arguments,
                             free,
                             __q06_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q06_execute);
}
