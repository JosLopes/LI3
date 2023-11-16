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
  * @return `NULL` on failure, a pointer to a `uint64_t` hotel ID otherwise.
  */
void *__q04_parse_arguments(char **argv, size_t argc) {
    if (argc != 1)
        return NULL;

    size_t length = strlen(argv[0]);
    if (length > 3) { /* Skip "HTL" before any reservation ID */
        uint64_t id;
        int      retval = int_utils_parse_positive(&id, argv[0] + 3);

        if (retval)
            return NULL; /* Non-HTL**** hotel */

        uint64_t *id_ptr = malloc(sizeof(uint64_t));
        if (id_ptr)
            *id_ptr = id;
        return id_ptr;
    } else {
        return NULL; /* Non-HTL**** hotel */
    }
}

/**
  * @brief Frees the value returned by ::__q04_parse_arguments.
  * @param argument_data Value returned by ::__q04_parse_arguments.
  */
void __q04_free_query_instance_argument_data(void *argument_data) {
    free(argument_data);
}

/**
  * @brief   Data passed to ::__q04_generate_statistics_foreach_reservation.
  * @details Auxiliary method for ::__q04_generate_statistics.
  */
typedef struct {
    GArray     *hotels_to_hash;
    GHashTable *hotel_reservations;
} q04_foreach_reservation_data_t;

/**
  * @brief Frees a `GArray` of reservations.
  * @param data `GArray` to be freed.
  */
void __q04_free_reservations_array(gpointer data) {
    g_array_free((GArray *) data, TRUE);
}

/**
 * @brief   A comparison function for sorting an `GArray` of `uint64_t`s.
 * @details Auxiliary method for ::__q04_generate_statistics.
 */
gint __q04_generate_statistics_uint64_compare_func(gconstpointer a, gconstpointer b) {
    return ((int64_t) * (uint64_t *) a) - ((int64_t) * (uint64_t *) b);
}

/**
  * @brief   A comparison function for sorting a `GArray` of reservations by date.
  * @details Auxiliary method for ::__q04_execute.
  */
gint __q04_sort_reservations_by_date(gconstpointer a, gconstpointer b) {
    reservation_t *reservation_a = *((reservation_t **) a);
    reservation_t *reservation_b = *((reservation_t **) b);

    if (date_diff(reservation_get_begin_date(reservation_a),
                  reservation_get_begin_date(reservation_b)) == 0) {
        return ((int64_t) * (uint64_t *) a) - ((int64_t) * (uint64_t *) b);
    } else {
        return date_diff(reservation_get_begin_date(reservation_b),
                         reservation_get_begin_date(reservation_a));
    }
}

/**
  * @brief   Callback for every reservation in the database.
  * @details Auxiliary method for ::__q04_generate_statistics.
  *
  * @param user_data    A ::q04_foreach_reservation_data_t.
  * @param reservation  A ::reservation_t in the database.
  *
  * @return `0` on success, `1` on allocation failure.
  */
int __q04_generate_statistics_foreach_reservation(void *user_data, reservation_t *reservation) {
    q04_foreach_reservation_data_t *foreach_data = (q04_foreach_reservation_data_t *) user_data;

    uint64_t hotel_id = reservation_get_hotel_id(reservation);

    if (!g_array_binary_search(foreach_data->hotels_to_hash,
                               &hotel_id,
                               __q04_generate_statistics_uint64_compare_func,
                               NULL))
        return 0; /* This hotel wasn't mentioned in queries */

    GArray *reservations =
        g_hash_table_lookup(foreach_data->hotel_reservations, GUINT_TO_POINTER(hotel_id));

    if (!reservations) {
        reservations = g_array_new(FALSE, FALSE, sizeof(reservation_t *));
        if (!reservations)
            return 1; /* Allocation failure */

        g_hash_table_insert(foreach_data->hotel_reservations,
                            GUINT_TO_POINTER(hotel_id),
                            reservations);
    }

    g_array_append_val(reservations, reservation);
    return 0;
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

    GArray *hotels_to_hash = g_array_new(FALSE, FALSE, sizeof(uint64_t));
    for (size_t i = 0; i < n; ++i) {
        uint64_t *hotel_id = (uint64_t *) query_instance_get_argument_data(instances);
        g_array_append_val(hotels_to_hash, *hotel_id);
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    g_array_sort(hotels_to_hash, __q04_generate_statistics_uint64_compare_func);

    GHashTable *hotel_reservations =
        g_hash_table_new_full(g_direct_hash,
                              g_direct_equal,
                              NULL,
                              (GDestroyNotify) __q04_free_reservations_array);
    q04_foreach_reservation_data_t callback_data = {.hotels_to_hash     = hotels_to_hash,
                                                    .hotel_reservations = hotel_reservations};

    if (reservation_manager_iter(database_get_reservations(database),
                                 __q04_generate_statistics_foreach_reservation,
                                 &callback_data)) { /* Allocation failure */

        g_hash_table_destroy(hotel_reservations);
        return NULL;
    }

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
 * @param statistics Statistical data generated by ::__q04_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success
 * @retval 1 Fatal failure.
 */
int __q04_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    uint64_t    hotel_id           = *((uint64_t *) query_instance_get_argument_data(instance));
    GHashTable *hotel_reservations = (GHashTable *) statistics;

    GArray *reservations = g_hash_table_lookup(hotel_reservations, GUINT_TO_POINTER(hotel_id));
    g_array_sort(reservations, __q04_sort_reservations_by_date);

    for (size_t i = 0; i < reservations->len; i++) {
        reservation_t *reservation = g_array_index(reservations, reservation_t *, i);

        if (!reservation)
            return 1; /* Fatal failure */

        uint64_t    reservation_id  = reservation_get_id(reservation);
        date_t      begin_date      = reservation_get_begin_date(reservation);
        date_t      end_date        = reservation_get_end_date(reservation);
        const char *user_id         = reservation_get_const_user_id(reservation);
        int         rating          = reservation_get_rating(reservation);
        int         price_per_night = reservation_get_price_per_night(reservation);
        int         city_tax        = reservation_get_city_tax(reservation);

        char begin_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];
        char end_date_str[DATE_SPRINTF_MIN_BUFFER_SIZE];

        date_sprintf(begin_date_str, begin_date);
        date_sprintf(end_date_str, end_date);

        int   reservation_days = date_diff(end_date, begin_date);
        float total_price      = price_per_night * reservation_days +
                            (price_per_night * reservation_days * 0.01) * city_tax;

        if (query_instance_get_formatted(instance)) {
            fprintf(output,
                    "--- %ld ---\nid: Book%010zu\nbegin_date: %s\nend_date: %s\nuser_id: "
                    "%s\nrating: %d\ntotal_price: %.3f\n\n",
                    i + 1,
                    reservation_id,
                    begin_date_str,
                    end_date_str,
                    user_id,
                    rating,
                    total_price);
        } else {
            fprintf(output,
                    "Book%010zu;%s;%s;%s;%d;%.3f\n",
                    reservation_id,
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
