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
 * @file  q03.c
 * @brief Implementation of methods in include/queries/q03.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @brief   Parses arguments for the third query.
 * @details Asserts that there's only one argument, an hotel identifier.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` on failure, a pointer to a hotel ID otherwise.
 */
void *__q03_parse_arguments(char **argv, size_t argc) {
    if (argc != 1)
        return NULL;

    hotel_id_t id;
    int        retval = hotel_id_from_string(&id, argv[0]);
    if (retval == 0) {
        hotel_id_t *id_ptr = malloc(sizeof(hotel_id_t));
        if (id_ptr)
            *id_ptr = id;

        return id_ptr;
    } else if (retval == 2) {
        /* TODO - have a way of communicating this failure */
        /* fprintf(stderr,
                  "Hotel ID \"%s\" not if format HTLXXXXX. This isn't supported by our program!\n",
                  token);*/
    }
    return NULL;
}

/**
 * @brief Frees the value returned by ::__q03_parse_arguments.
 * @param argument_data Value returned by ::__q03_parse_arguments.
 */
void __q03_free_query_instance_argument_data(void *argument_data) {
    free(argument_data);
}

/**
 * @struct q03_average_t
 * @brief Data structure containing the fields needed to calculate a flying average.
 *
 * @var q03_average_t::sum
 *     @brief Sum of all ratings checked until now.
 * @var q03_average_t::count
 *     @brief Number of reservations checked until now.
 */
typedef struct {
    uint64_t sum;
    size_t   count;
} q03_average_t;

/**
 * @struct q03_foreach_reservation_data_t
 * @brief  Data needed while iterating over reservations.
 *
 * @var q03_foreach_reservation_data_t::hotels_to_average
 *     @brief Sorted array of hotels (identifiers) whose average rating was queried.
 * @var q03_foreach_reservation_data_t::ratings_averages
 *     @brief Association between hotel identifiers and pointers to ::q03_average_t.
 */
typedef struct {
    GArray     *hotels_to_average;
    GHashTable *ratings_averages;
} q03_foreach_reservation_data_t;

/**
 * @brief   A comparison function for sorting an `GArray` of ::hotel_id_t.
 * @details Auxiliary method for ::__q03_generate_statistics.
 */
gint __q03_generate_statistics_hotel_id_compare_func(gconstpointer a, gconstpointer b) {
    return ((hotel_id_t) * (hotel_id_t *) a) - ((hotel_id_t) * (hotel_id_t *) b);
}

/**
 * @brief   A method called for each reservation, to consider it in averages being calculated.
 * @details An auxiliary method for ::__q03_generate_statistics.
 *
 * @param user_data   A pointer to a ::q03_foreach_reservation_data_t.
 * @param reservation Reservation being processed.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 */
int __q03_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q03_foreach_reservation_data_t *foreach_data = (q03_foreach_reservation_data_t *) user_data;

    hotel_id_t hotel_id = reservation_get_hotel_id(reservation);
    uint8_t    rating   = reservation_get_rating(reservation);

    if (!g_array_binary_search(foreach_data->hotels_to_average,
                               &hotel_id,
                               __q03_generate_statistics_hotel_id_compare_func,
                               NULL))
        return 0; /* This hotel wasn't mentioned in queries */

    q03_average_t *avg =
        g_hash_table_lookup(foreach_data->ratings_averages, GUINT_TO_POINTER(hotel_id));

    if (!avg) {
        avg = malloc(sizeof(q03_average_t));
        if (!avg)
            return 1; /* Allocation failure */

        avg->sum   = 0;
        avg->count = 0;
        g_hash_table_insert(foreach_data->ratings_averages, GUINT_TO_POINTER(hotel_id), avg);
    }

    avg->sum += rating;
    avg->count++;
    return 0;
}

/**
 * @brief Generates statistical data for queries of type 3.
 *
 * @param database  Database, to iterate through reservations.
 * @param instances Query instances that will need to be executed.
 * @param n         Number of query instances that will need to be executed.
 *
 * @return A `GHashTable` that associates hotel identifiers with ::q03_average_t for those hotels,
 *         or `NULL` on failure.
 */
void *__q03_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {

    /*
     * Generate an array of all hotel IDs that will be asked for in queries, to avoid storing
     * rating information about all hotels.
     */
    GArray *hotels_to_average = g_array_new(FALSE, FALSE, sizeof(hotel_id_t));
    for (size_t i = 0; i < n; ++i) {
        hotel_id_t *hotel_id = (hotel_id_t *) query_instance_get_argument_data(instances);
        g_array_append_val(hotels_to_average, *hotel_id);
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    g_array_sort(hotels_to_average, __q03_generate_statistics_hotel_id_compare_func);

    /* Iterate through all reservations to calculate rating averages. */
    GHashTable *ratings_averages =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) free);
    q03_foreach_reservation_data_t callback_data = {.hotels_to_average = hotels_to_average,
                                                    .ratings_averages  = ratings_averages};

    if (reservation_manager_iter(database_get_reservations(database),
                                 __q03_generate_statistics_foreach_reservation,
                                 &callback_data)) { /* Allocation failure */

        g_array_free(hotels_to_average, TRUE);
        g_hash_table_destroy(ratings_averages);
        return NULL;
    }

    g_array_free(hotels_to_average, TRUE);
    return ratings_averages;
}

/**
 * @brief Frees statistical data generated by ::__q03_generate_statistics.
 * @param statistics Data generated by ::__q03_generate_statistics.
 */
void __q03_free_statistics(void *statistics) {
    g_hash_table_destroy(statistics);
}

/**
 * @brief Method called to execute a query of type 3.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q03_generate_statistics).
 * @param statistics Statistical data generated by ::__q03_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (will only happen if a cosmic ray flips some bit in your memory).
 */
int __q03_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    GHashTable *ratings_averages = (GHashTable *) statistics;
    hotel_id_t  hotel_id         = *(hotel_id_t *) query_instance_get_argument_data(instance);

    q03_average_t *avg = g_hash_table_lookup(ratings_averages, GUINT_TO_POINTER(hotel_id));
    if (!avg) {
        fprintf(stderr, "Bad statistical data in query 3! This should not happen!\n");
        return 1;
    }

    if (query_instance_get_formatted(instance)) {
        fprintf(output, "--- 1 ---\nrating: %.3f\n", (double) avg->sum / (double) avg->count);
    } else {
        fprintf(output, "%.3f\n", (double) avg->sum / (double) avg->count);
    }

    return 0;
}

query_type_t *q03_create(void) {
    return query_type_create(__q03_parse_arguments,
                             __q03_free_query_instance_argument_data,
                             __q03_generate_statistics,
                             __q03_free_statistics,
                             __q03_execute);
}
