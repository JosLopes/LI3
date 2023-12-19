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
 * @file  q067c
 * @brief Implementation of methods in include/queries/q07.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

void *__q07_parse_arguments(char **argv, size_t argc) {
    if (argc != 1)
        return NULL;

    /* Parse number of flights */
    uint64_t n;
    int      retval = int_utils_parse_positive(&n, argv[0]);
    if (retval) {
        return NULL; /* Invalid N format */
    }

    uint64_t *n_ptr = malloc(sizeof(uint64_t));
    if (n_ptr) {
        *n_ptr = n;
        return n_ptr;
    } else {
        return NULL;
    }
}

void __q07_free_query_instance_argument_data(void *argument_data) {
    free(argument_data);
}

void __q07_destroy_list(gpointer data) {
    g_list_free((GList *) data);
}

gint __q07_compare_ints(gconstpointer a, gconstpointer b) {
    return GPOINTER_TO_UINT(a) - GPOINTER_TO_UINT(b);
}

gint __q07_compare_airports_median(gconstpointer a, gconstpointer b, gpointer user_data) {
    GHashTable *airports_median = (GHashTable *) user_data;

    // if the median is equal, compare the airport code
    if (GPOINTER_TO_UINT(g_hash_table_lookup(airports_median, b)) ==
        GPOINTER_TO_UINT(g_hash_table_lookup(airports_median, a))) {
        return GPOINTER_TO_UINT(b) - GPOINTER_TO_UINT(a);
    }
    return GPOINTER_TO_UINT(g_hash_table_lookup(airports_median, b)) -
           GPOINTER_TO_UINT(g_hash_table_lookup(airports_median, a));
}

int __q07_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    GHashTable *airport_delays = (GHashTable *) user_data;

    /* Get flight airport code, scheduled departure and real departure */
    airport_code_t  airport_code        = flight_get_origin(flight);
    date_and_time_t scheduled_departure = flight_get_schedule_departure_date(flight);
    date_and_time_t real_departure      = flight_get_real_departure_date(flight);

    /* Get subtraction of real_departure with scheduled_departure. */
    uint64_t delay = date_and_time_diff(real_departure, scheduled_departure);

    /* Insert delays in the hash table */
    if (g_hash_table_contains(airport_delays, GUINT_TO_POINTER(airport_code))) {
        GList *delays = g_hash_table_lookup(airport_delays, GUINT_TO_POINTER(airport_code));
        delays        = g_list_insert_sorted(delays, GUINT_TO_POINTER(delay), __q07_compare_ints);
        g_hash_table_replace(airport_delays, GUINT_TO_POINTER(airport_code), delays);
    } else {
        GList *delays = NULL;
        delays        = g_list_insert_sorted(delays, GUINT_TO_POINTER(delay), __q07_compare_ints);
        g_hash_table_insert(airport_delays, GUINT_TO_POINTER(airport_code), delays);
    }

    return 0;
}

void *__q07_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) instances;
    (void) n;

    GHashTable *airport_delays  = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    GHashTable *airport_medians = g_hash_table_new(g_direct_hash, g_direct_equal);

    flight_manager_iter(database_get_flights(database),
                        __q07_generate_statistics_foreach_flight,
                        airport_delays);

    // iterate throught the hashtable and calculate the median
    GHashTableIter iter;
    gpointer       key, value;
    g_hash_table_iter_init(&iter, airport_delays);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        GList   *delays = (GList *) value;
        uint64_t median;
        if (g_list_length(delays) % 2 == 0) {
            median = (GPOINTER_TO_UINT(g_list_nth_data(delays, g_list_length(delays) / 2)) +
                      GPOINTER_TO_UINT(g_list_nth_data(delays, g_list_length(delays) / 2 - 1))) /
                     2;
        } else {
            median = GPOINTER_TO_UINT(g_list_nth_data(delays, g_list_length(delays) / 2));
        }
        g_hash_table_insert(airport_medians, key, GUINT_TO_POINTER(median));
    }

    g_hash_table_destroy(airport_delays);
    return airport_medians;
}

void __q07_free_statistics(void *statistics) {
    g_hash_table_destroy(statistics);
}

int __q07_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    uint64_t n = *(uint64_t *) query_instance_get_argument_data(instance);

    GHashTable *airport_delays = (GHashTable *) statistics;

    // sort the hash table by the median
    GList *sorted_airports = g_hash_table_get_keys(airport_delays);
    sorted_airports =
        g_list_sort_with_data(sorted_airports, __q07_compare_airports_median, airport_delays);

    // print the top n airports
    for (size_t i = 0; i < n && i < g_list_length(sorted_airports); i++) {
        airport_code_t airport_code = GPOINTER_TO_UINT(g_list_nth_data(sorted_airports, i));
        uint64_t       median =
            GPOINTER_TO_UINT(g_hash_table_lookup(airport_delays, GUINT_TO_POINTER(airport_code)));

        char *airport_code_str = malloc(4 * sizeof(char));
        airport_code_sprintf(airport_code_str, airport_code);
        fprintf(output, "%s;%lu\n", airport_code_str, median);

        free(airport_code_str);
    }

    g_list_free(sorted_airports);
    return 0;
}

query_type_t *q07_create(void) {
    return query_type_create(__q07_parse_arguments,
                             __q07_free_query_instance_argument_data,
                             __q07_generate_statistics,
                             __q07_free_statistics,
                             __q07_execute);
}
