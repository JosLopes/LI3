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

typedef struct {
    airport_code_t airport_code;
    uint64_t       median;
} __q07_airport_median;

gint __q07_compare_ints(gconstpointer a, gconstpointer b) {
    return GPOINTER_TO_UINT(a) - GPOINTER_TO_UINT(b);
}

gint __q07_compare_airports_median(gconstpointer a, gconstpointer b) {
    __q07_airport_median *airport_median_a = (__q07_airport_median *) a;
    __q07_airport_median *airport_median_b = (__q07_airport_median *) b;

    if (airport_median_a->median == airport_median_b->median) {
        char *airport_code_a = malloc(3 * sizeof(char));
        char *airport_code_b = malloc(3 * sizeof(char));
        airport_code_sprintf(airport_code_a, airport_median_a->airport_code);
        airport_code_sprintf(airport_code_b, airport_median_b->airport_code);

        int retval = strcmp(airport_code_a, airport_code_b);

        free(airport_code_a);
        free(airport_code_b);
        return retval;
    } else {
        return airport_median_b->median - airport_median_a->median;
    }
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

    GHashTable *airport_delays  = g_hash_table_new(g_direct_hash, g_direct_equal);
    GArray     *airport_medians = g_array_new(FALSE, FALSE, sizeof(__q07_airport_median));

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
        // create airport median struct and add it to airport_medians array
        __q07_airport_median airport_median = {GPOINTER_TO_UINT(key), median};
        g_array_append_val(airport_medians, airport_median);
    }

    g_hash_table_destroy(airport_delays);
    return airport_medians;
}

void __q07_free_statistics(void *statistics) {
    GArray *airport_medians = (GArray *) statistics;
    g_array_free(airport_medians, TRUE);
}

int __q07_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    uint64_t n = *(uint64_t *) query_instance_get_argument_data(instance);

    GArray *airport_medians = (GArray *) statistics;

    // sort the array by median and airport code
    g_array_sort(airport_medians, __q07_compare_airports_median);

    // print the first n airports and dont go over length of array
    char *airport_code = malloc(3 * sizeof(char));
    for (size_t i = 0; i < n && i < airport_medians->len; i++) {
        __q07_airport_median *airport_median =
            &g_array_index(airport_medians, __q07_airport_median, i);

        airport_code_sprintf(airport_code, airport_median->airport_code);
        fprintf(output, "%s;%lu\n", airport_code, airport_median->median);
    }
    free(airport_code);

    return 0;
}

query_type_t *q07_create(void) {
    return query_type_create(__q07_parse_arguments,
                             __q07_free_query_instance_argument_data,
                             __q07_generate_statistics,
                             __q07_free_statistics,
                             __q07_execute);
}
