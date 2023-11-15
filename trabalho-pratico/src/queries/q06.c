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
 * @brief Implementation of methods in include/queries/q03.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @struct q06_parse_arguments_t
 * @brief A structure to hold the parsed arguments for a query of type 6.
 *
 * @var q06_parse_arguments_t::year
 *     @brief The year to consider.
 * @var q06_parse_arguments_t::n
 *    @brief The number of airports to display.
 */
typedef struct {
    uint16_t year;
    int      n;
} q06_parse_arguments_t;

/**
 * @brief Parses the arguments for a query of type 6.
 * @details Asserts that there are exactly two arguments, a year and a number of airports to display.
 *
 * @param argv Arguments vector.
 * @param argc Arguments count.
 *
 * @return A ::q06_parse_arguments_t with the parsed arguments, or `NULL` on failure.
 */
void *__q06_parse_arguments(char **argv, size_t argc) {
    if (argc != 2)
        return NULL;

    q06_parse_arguments_t *args = malloc(sizeof(q06_parse_arguments_t));
    if (!args)
        return NULL;

    /* Parse year */
    size_t length = strlen(argv[0]);
    if (length == 4) {
        uint64_t year;
        int      retval = int_utils_parse_positive(&year, argv[0]);

        if (retval)
            return NULL; /* Invalid year format */

        args->year = year;
    } else {
        return NULL; /* Invalid year format */
    }

    /* Parse N */
    length = strlen(argv[1]);
    if (length > 0) {
        uint64_t n;
        int      retval = int_utils_parse_positive(&n, argv[1]);

        if (retval)
            return NULL; /* Invalid N format */

        args->n = n;
    } else {
        return NULL; /* N bellow zero */
    }

    return args;
}

/**
 * @brief Frees the value returned by ::__q06_parse_arguments.
 * @param argument_data Value returned by ::__q06_parse_arguments.
 */
void __q06_free_query_instance_argument_data(void *argument_data) {
    free(argument_data);
}

/**
 * @struct q06_key_t
 * @brief A structure to hold the key for a hash table entry.
 *
 * @var q06_key_t::year
 *     @brief The year.
 * @var q06_key_t::code
 *    @brief The airport code.
 */
typedef struct {
    uint16_t       year;
    airport_code_t code;
} q06_key_t;

/**
 * @brief Hash function for ::q06_key_t.
 *
 * @param key The key to hash.
 *
 * @return The hash value.
 */
guint __q06_hash_key(gconstpointer key) {
    q06_key_t *key_ptr = (q06_key_t *) key;

    return key_ptr->year + key_ptr->code;
}

/**
 * @brief Comparison function for ::q06_key_t.
 *
 * @param a The first key.
 * @param b The second key.
 *
 * @return `TRUE` if the keys are equal, `FALSE` otherwise.
 */
gboolean __q06_compare_keys(gconstpointer a, gconstpointer b) {
    q06_key_t *key_a = (q06_key_t *) a;
    q06_key_t *key_b = (q06_key_t *) b;

    return key_a->year == key_b->year && key_a->code == key_b->code;
}

/**
 * @brief Method called for each flight, to generate statistical data.
 * @details An auxiliary method for ::__q06_generate_statistics.
 *
 * @param user_data     A `GHashTable` that associates ::q06_key_t with `int`s.
 * @param flight        The flight to consider.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (will only happen if a cosmic ray flips some bit in your memory).
 */
int __q06_generate_statistics_foreach_flight(void *user_data, flight_t *flight) {
    GHashTable     *airline_count = (GHashTable *) user_data;
    date_and_time_t date_and_time = flight_get_schedule_departure_date(flight);
    date_t          date          = date_and_time_get_date(date_and_time);
    uint16_t        year          = date_get_year(date);

    airport_code_t origin_airport_code      = flight_get_origin(flight);
    airport_code_t destination_airport_code = flight_get_destination(flight);

    q06_key_t *origin_key = malloc(sizeof(q06_key_t));
    if (!origin_key)
        return 1; /* Allocation failure */
    origin_key->year = year;
    origin_key->code = origin_airport_code;

    q06_key_t *destination_key = malloc(sizeof(q06_key_t));
    if (!destination_key)
        return 1; /* Allocation failure */
    destination_key->year = year;
    destination_key->code = destination_airport_code;

    int num_passangers = flight_get_number_of_passengers(flight);

    int *origin_count = g_hash_table_lookup(airline_count, origin_key);
    if (!origin_count) {
        origin_count = malloc(sizeof(int));
        if (!origin_count)
            return 1; /* Allocation failure */

        *origin_count = num_passangers;
        g_hash_table_insert(airline_count, origin_key, origin_count);
    } else {
        *origin_count += num_passangers;
    }

    int *destination_count = g_hash_table_lookup(airline_count, destination_key);
    if (!destination_count) {
        destination_count = malloc(sizeof(int));
        if (!destination_count)
            return 1; /* Allocation failure */

        *destination_count = num_passangers;
        g_hash_table_insert(airline_count, destination_key, destination_count);
    } else {
        *destination_count += num_passangers;
    }

    return 0;
}

/**
 * @brief Generates statistical data for queries of type 6.
 * @details Generates a `GHashTable` that associates ::q06_key_t with `int`s.
 *
 * @param database   Database to get data from.
 * @param instances  Array of query instances to be executed.
 * @param n          Number of query instances to be executed.
 *
 * @return A `GHashTable` that associates ::q06_key_t with `int`s, or `NULL` on failure.
 */
void *__q06_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) instances;
    (void) n;

    GHashTable *airline_count =
        g_hash_table_new_full(__q06_hash_key, __q06_compare_keys, NULL, (GDestroyNotify) free);

    if (flight_manager_iter(database_get_flights(database),
                            __q06_generate_statistics_foreach_flight,
                            airline_count)) { /* Allocation failure */

        g_hash_table_destroy(airline_count);
        return NULL;
    }

    return airline_count;
}

/**
 * @brief Frees the value returned by ::__q06_generate_statistics.
 * @param statistics Value returned by ::__q06_generate_statistics.
 */
void __q06_free_statistics(void *statistics) {
    g_hash_table_destroy(statistics);
}

/**
 * @struct q06_array_item_t
 * @brief A structure to hold an item in the array of airport passanger counts.
 *
 * @var q06_array_item_t::key
 *     @brief The key.
 * @var q06_array_item_t::count
 *    @brief The number of passangers.
 */
typedef struct {
    q06_key_t key;
    int       count;
} q06_array_item_t;

/**
 * @brief Method called for each airport-count pair, to generate the array of airport passanger counts.
 * @details An auxiliary method for ::__q06_execute.
 *
 * @param key       The key.
 * @param value     The number of passangers.
 * @param user_data The array of airport passanger counts.
 */
void __q06_foreach_airport_count(gpointer key, gpointer value, gpointer user_data) {
    GArray *airport_array = (GArray *) user_data;

    q06_array_item_t item;
    item.key   = *((q06_key_t *) key);
    item.count = *((int *) value);

    g_array_append_val(airport_array, item);
}

/**
 * @brief Comparison function for ::q06_array_item_t.
 *
 * @param a The first item.
 * @param b The second item.
 *
 * @return `TRUE` if the first item is greater than the second, `FALSE` otherwise.
 */
gint __q06_sort_airports_by_count(gconstpointer a, gconstpointer b) {
    q06_array_item_t *item_a = (q06_array_item_t *) a;
    q06_array_item_t *item_b = (q06_array_item_t *) b;

    if (item_b->count == item_a->count) {
        return item_b->key.code - item_a->key.code;
    } else {
        return item_b->count - item_a->count;
    }
}

/**
 * @brief Executes a query of type 6.
 * @details Prints the top N airports with the most passangers in a given year.
 *
 * @param database   Database to get data from.
 * @param statistics Statistics generated by ::__q06_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     File to write output to.
 *
 * @retval 0 Success
 */
int __q06_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    GHashTable            *airport_count = (GHashTable *) statistics;
    GArray                *airport_array = g_array_new(FALSE, FALSE, sizeof(q06_array_item_t));
    q06_parse_arguments_t *args =
        (q06_parse_arguments_t *) query_instance_get_argument_data(instance);

    uint16_t year = args->year;
    int      n    = args->n;

    g_hash_table_foreach(airport_count, __q06_foreach_airport_count, airport_array);
    g_array_sort(airport_array, __q06_sort_airports_by_count);

    for (size_t i = 0; i < airport_array->len; i++) {
        q06_array_item_t *item = &g_array_index(airport_array, q06_array_item_t, i);

        char airport_code_str[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
        airport_code_sprintf(airport_code_str, item->key.code);

        if (item->key.year != year)
            continue;
        if (n-- == 0)
            break;

        if (query_instance_get_formatted(instance)) {
            fprintf(output,
                    "--- %d ---\nname: %s\npassangers: %d\n\n",
                    10 - n,
                    airport_code_str,
                    item->count);
        } else {
            fprintf(output, "%s;%d\n", airport_code_str, item->count);
        }
    }

    return 0;
}

query_type_t *q06_create(void) {
    return query_type_create(__q06_parse_arguments,
                             __q06_free_query_instance_argument_data,
                             __q06_generate_statistics,
                             __q06_free_statistics,
                             __q06_execute);
}
