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
  * @struct q04_foreach_reservation_data_t
  * @brief  Data needed while iterating over reservations.
  *
  * @var q04_foreach_reservation_data_t::reservation_ids
  *     @brief Array of reservations booked in hotel_id, that will be sorted by date.
  * @var q04_foreach_reservation_data_t::hotel_id
  *     @brief Hotel ID to be used to filter reservations.
  */
typedef struct {
    GArray   *reservation_ids;
    uint64_t *hotel_id;
} q04_foreach_reservation_data_t;

/**
  * @brief   A comparison function for sorting a `GArray` of reservation IDs by date.
  * @details Auxiliary method for ::__q04_execute.
  */
gint __q04_sort_reservations_by_date(gconstpointer a, gconstpointer b, gpointer user_data) {
    uint64_t               reservation_a_id = *((uint64_t *) a);
    uint64_t               reservation_b_id = *((uint64_t *) b);
    reservation_manager_t *manager          = (reservation_manager_t *) user_data;

    reservation_t *reservation_a = reservation_manager_get_by_id(manager, reservation_a_id);
    reservation_t *reservation_b = reservation_manager_get_by_id(manager, reservation_b_id);

    if (date_diff(reservation_get_begin_date(reservation_a),
                  reservation_get_begin_date(reservation_b)) == 0) {
        return ((int64_t) * (uint64_t *) a) - ((int64_t) * (uint64_t *) b);
    } else {
        return date_diff(reservation_get_begin_date(reservation_b),
                         reservation_get_begin_date(reservation_a));
    }
}

/**
  * @brief   Callback function for ::reservation_manager_iter.
  * @details Fills a `GArray` with reservation IDs that match the hotel ID.
  */
int __q04_fill_array_with_reservations(void *user_data, reservation_t *reservation) {
    q04_foreach_reservation_data_t *foreach_data = (q04_foreach_reservation_data_t *) user_data;

    uint64_t reservation_id = reservation_get_id(reservation);

    if (reservation_get_hotel_id(reservation) == *((int *) foreach_data->hotel_id))
        g_array_append_val(foreach_data->reservation_ids, reservation_id);

    return 0;
}

/**
  * @brief Method called to execute a query of type 4.
  *
  * @param database   Database to get data from.
  * @param statistics Statistical data (not used, as it isn't needed in this query).
  * @param instance   Query instance to be executed.
  * @param output     Where to write the query's result to.
  *
  * @retval 0 Success
  * @retval 1 Fatal failure
  */
int __q04_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) statistics;

    GArray   *reservation_ids = g_array_new(FALSE, FALSE, sizeof(uint64_t));
    uint64_t *hotel_id        = (uint64_t *) query_instance_get_argument_data(instance);

    q04_foreach_reservation_data_t callback_data = {.reservation_ids = reservation_ids,
                                                    .hotel_id        = hotel_id};

    if (reservation_manager_iter(database_get_reservations(database),
                                 __q04_fill_array_with_reservations,
                                 &callback_data)) {

        g_array_free(reservation_ids, TRUE);
        return 1;
    }

    g_array_sort_with_data(reservation_ids,
                           __q04_sort_reservations_by_date,
                           (gpointer *) database_get_reservations(database));

    for (size_t i = 0; i < reservation_ids->len; i++) {
        uint64_t       current_reservation_id = g_array_index(reservation_ids, uint64_t, i);
        reservation_t *current_reservation =
            reservation_manager_get_by_id(database_get_reservations(database),
                                          current_reservation_id);

        date_t      begin_date      = reservation_get_begin_date(current_reservation);
        date_t      end_date        = reservation_get_end_date(current_reservation);
        const char *user_id         = reservation_get_const_user_id(current_reservation);
        int         rating          = reservation_get_rating(current_reservation);
        int         price_per_night = reservation_get_price_per_night(current_reservation);
        int         city_tax        = reservation_get_city_tax(current_reservation);

        char begin_date_str[11];
        char end_date_str[11];
        date_sprintf(begin_date_str, begin_date);
        date_sprintf(end_date_str, end_date);

        int   reservation_days = date_diff(end_date, begin_date);
        float total_price      = price_per_night * reservation_days +
                            (price_per_night * reservation_days * 0.01) * city_tax;

        fprintf(output,
                "Book%010zu;%s;%s;%s;%d;%.3f\n",
                current_reservation_id,
                begin_date_str,
                end_date_str,
                user_id,
                rating,
                total_price);
    }

    return 0;
}

query_type_t *q04_create(void) {
    return query_type_create(__q04_parse_arguments,
                             __q04_free_query_instance_argument_data,
                             NULL,
                             NULL,
                             __q04_execute);
}