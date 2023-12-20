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
  * @file  q04.c
  * @brief Implementation of methods in include/queries/q04.h
  */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
  * @brief   Parses arguments for the fourth query.
  * @details Asserts that there's only one argument, the hotel identifier.
  *
  * @param argv Values of the arguments.
  * @param argc Number of arguments.
  *
  * @return `NULL` on failure, a pointer to a hotel ID otherwise.
  */
void *__q04_parse_arguments(char **argv, size_t argc) {
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
  * @brief Frees the value returned by ::__q04_parse_arguments.
  * @param argument_data Value returned by ::__q04_parse_arguments.
  */
void __q04_free_query_instance_argument_data(void *argument_data) {
    free(argument_data);
}

/**
  * @struct q04_foreach_reservation_data_t
  * @brief   Data passed to ::__q04_generate_statistics_foreach_reservation.
  * @details Auxiliary data type for ::__q04_generate_statistics.
  *
  * @var q04_foreach_reservation_data_t::hotels_to_consider
  *     @brief Hotels whose reservations need to be registered.
  * @var q04_foreach_reservation_data_t::hotel_reservations
  *     @brief Associates hotel identifiers to `GPtrArray`s of hotel reservations.
  */
typedef struct {
    GArray     *hotels_to_consider;
    GHashTable *hotel_reservations;
} q04_foreach_reservation_data_t;

/**
  * @brief Frees a `GPtrArray` of reservations, used as a `GDestroyNotify` to automatically free a
  *        hash table.
  * @param data `GAPtrArray` to be freed.
  */
void __q04_free_reservations_array(gpointer data) {
    g_ptr_array_free((GPtrArray *) data, TRUE);
}

/**
 * @brief   A comparison function for sorting and performing binary search on an `GArray`s of
 *          ::hotel_id_t.
 * @details Auxiliary method for ::__q04_generate_statistics_foreach_reservation, itself an
 *          auxiliary method for ::__q04_generate_statistics.
 */
gint __q04_generate_statistics_hotel_id_compare_func(gconstpointer a, gconstpointer b) {
    return ((hotel_id_t) * (hotel_id_t *) a) - ((hotel_id_t) * (hotel_id_t *) b);
}

/**
  * @brief   Callback for every reservation in the database, that adds it to its corresponing hotel
  *          in @p user_data.
  * @details Auxiliary method for ::__q04_generate_statistics.
  *
  * @param user_data    A ::q04_foreach_reservation_data_t.
  * @param reservation  A ::reservation_t in the database.
  *
  * @retval `0` Always successful.
  */
int __q04_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q04_foreach_reservation_data_t *foreach_data = (q04_foreach_reservation_data_t *) user_data;

    hotel_id_t hotel_id = reservation_get_hotel_id(reservation);
    if (!g_array_binary_search(foreach_data->hotels_to_consider,
                               &hotel_id,
                               __q04_generate_statistics_hotel_id_compare_func,
                               NULL))
        return 0; /* This hotel wasn't mentioned in queries */

    GPtrArray *reservations =
        g_hash_table_lookup(foreach_data->hotel_reservations, GUINT_TO_POINTER(hotel_id));

    if (!reservations) {
        reservations = g_ptr_array_new();
        g_hash_table_insert(foreach_data->hotel_reservations,
                            GUINT_TO_POINTER(hotel_id),
                            reservations);
    }

    /* TODO - find a way of not keeping the const */
    g_ptr_array_add(reservations, (reservation_t *) reservation);
    return 0;
}

/**
  * @brief   A comparison function for sorting a `GPtrArray` of reservations by date.
  * @details Auxiliary method for ::__q04_generate_statistics_sort_each_array, itself an
  *          auxiliary method for ::__q04_generate_statistics.
  */
gint __q04_sort_reservations_by_date(gconstpointer a, gconstpointer b) {
    const reservation_t *reservation_a = *((const reservation_t **) a);
    const reservation_t *reservation_b = *((const reservation_t **) b);

    int64_t diff = date_diff(reservation_get_begin_date(reservation_b),
                             reservation_get_begin_date(reservation_a));

    if (diff == 0)
        return reservation_get_id(reservation_a) - reservation_get_id(reservation_b);
    else
        return diff;
}

/**
 * @brief Sorts each array of reservations associated to a hotel. Iteration function for a
 *        hash table.
 *
 * @param key       Hotel identifier (as a pointer), not used.
 * @param value     `GPtrArray` of pointers to ::reservation_t to be sorted.
 * @param user_data Not used (`NULL` is provided).
 */
void __q04_generate_statistics_sort_each_array(gpointer key, gpointer value, gpointer user_data) {
    (void) key;
    (void) user_data;

    g_ptr_array_sort((GPtrArray *) value, __q04_sort_reservations_by_date);
}

/**
  * @brief Generates statistical data for queries of type 4.
  *
  * @param database   Database, to iterate through reservations.
  * @param instances  Instances of the query.
  * @param n          Number of instances.
  *
  * @return A `GHashTable` associating hotel identifiers to `GArray`s of `reservation_t`s.
  */
void *__q04_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) instances;
    (void) n;

    /* TODO - use set (hash table) for better performance */
    GArray *hotels_to_consider = g_array_new(FALSE, FALSE, sizeof(hotel_id_t));
    for (size_t i = 0; i < n; ++i) {
        hotel_id_t *hotel_id = (hotel_id_t *) query_instance_get_argument_data(instances);
        g_array_append_val(hotels_to_consider, *hotel_id);
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    g_array_sort(hotels_to_consider, __q04_generate_statistics_hotel_id_compare_func);

    GHashTable *hotel_reservations =
        g_hash_table_new_full(g_direct_hash,
                              g_direct_equal,
                              NULL,
                              (GDestroyNotify) __q04_free_reservations_array);
    q04_foreach_reservation_data_t callback_data = {.hotels_to_consider = hotels_to_consider,
                                                    .hotel_reservations = hotel_reservations};

    reservation_manager_iter(database_get_reservations(database),
                             __q04_generate_statistics_foreach_reservation,
                             &callback_data);

    g_hash_table_foreach(hotel_reservations, __q04_generate_statistics_sort_each_array, NULL);

    g_array_free(hotels_to_consider, TRUE);
    return hotel_reservations;
}

/**
 * @brief Frees statistical data generated by ::__q04_generate_statistics.
 * @param statistics Data generated by ::__q04_generate_statistics.
 */
void __q04_free_statistics(void *statistics) {
    g_hash_table_destroy(statistics);
}

/**
 * @brief Method called to execute a query of type 4.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q04_generate_statistics).
 * @param statistics Statistical data generated by ::__q04_generate_statistics (a hashtable that
 *                   associates identifiers of hotels to `GArray`s of reservations).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (shouldn't happen nunder normal circumstances).
 */
int __q04_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    hotel_id_t hotel_id = *((hotel_id_t *) query_instance_get_argument_data(instance));
    GPtrArray *reservations =
        g_hash_table_lookup((GHashTable *) statistics, GUINT_TO_POINTER(hotel_id));
    if (!reservations)
        return 1; /* Only happens on bad statistics, which itself shouldn't happen. */

    for (size_t i = 0; i < reservations->len; i++) {
        reservation_t *reservation = g_ptr_array_index(reservations, i);

        reservation_id_t reservation_id = reservation_get_id(reservation);
        date_t           begin_date     = reservation_get_begin_date(reservation);
        date_t           end_date       = reservation_get_end_date(reservation);
        const char      *user_id        = reservation_get_const_user_id(reservation);
        uint8_t          rating         = reservation_get_rating(reservation);
        double           total_price    = reservation_calculate_price(reservation);

        char begin_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
        char end_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
        char reservation_id_str[RESERVATION_ID_SPRINTF_MIN_BUFFER_SIZE];
        date_sprintf(begin_date_str, begin_date);
        date_sprintf(end_date_str, end_date);
        reservation_id_sprintf(reservation_id_str, reservation_id);

        if (query_instance_get_formatted(instance)) {
            fprintf(output,
                    "--- %ld ---\nid: %s\nbegin_date: %s\nend_date: %s\nuser_id: %s\n"
                    "rating: %" PRIu8 "\ntotal_price: %.3f\n",
                    i + 1,
                    reservation_id_str,
                    begin_date_str,
                    end_date_str,
                    user_id,
                    rating,
                    total_price);

            if (i != reservations->len - 1)
                fputc('\n', output);
        } else {
            fprintf(output,
                    "%s;%s;%s;%s;%" PRIu8 ";%.3f\n",
                    reservation_id_str,
                    begin_date_str,
                    end_date_str,
                    user_id,
                    rating,
                    total_price);
        }
    }

    return 0;
}

query_type_t *q04_create(void) {
    return query_type_create(__q04_parse_arguments,
                             __q04_free_query_instance_argument_data,
                             __q04_generate_statistics,
                             __q04_free_statistics,
                             __q04_execute);
}
