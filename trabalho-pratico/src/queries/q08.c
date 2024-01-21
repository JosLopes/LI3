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
 * @file  q08.c
 * @brief Implementation of methods in include/queries/q08.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/q08.h"
#include "queries/query_instance.h"

/**
 * @struct q08_parsed_arguments_t
 * @brief  Data needed for the execution of a query of type 8.
 *
 * @var q08_parsed_arguments_t::hotel_id
 *     @brief Hotel identifier to filter by.
 * @var q08_parsed_arguments_t::begin_date
 *     @brief Beginning date for date range filter.
 * @var q08_parsed_arguments_t::end_date
 *     @brief End date for date range filter.
 */
typedef struct {
    hotel_id_t hotel_id;
    date_t     begin_date;
    date_t     end_date;
} q08_parsed_arguments_t;

/**
 * @brief   Parses arguments for query 8.
 * @details Asserts that there's three arguments, an hotel identifier, and two dates.
 *
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 *
 * @return `NULL` on failure, a pointer to a `q08_airport_data_t` otherwise.
 */
void *__q08_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 3)
        return NULL;

    q08_parsed_arguments_t *parsed_arguments = malloc(sizeof(q08_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    int hotel_retval      = hotel_id_from_string(&parsed_arguments->hotel_id, argv[0]);
    int begin_date_retval = date_from_string(&parsed_arguments->begin_date, argv[1]);
    int end_date_retval   = date_from_string(&parsed_arguments->end_date, argv[2]);

    if (hotel_retval || begin_date_retval || end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }

    return parsed_arguments;
}

void *__q08_clone_arguments(const void *args_data) {
    const q08_parsed_arguments_t *args  = args_data;
    q08_parsed_arguments_t       *clone = malloc(sizeof(q08_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q08_parsed_arguments_t));
    return clone;
}

/**
 * @struct q08_foreach_reservation_data_t
 * @brief  Data needed while iterating over reservations.
 *
 * @var q08_foreach_reservation_data_t::filter_data
 *     @brief Array of pointers to ::q08_parsed_arguments_t.
 * @var q08_foreach_reservation_data_t::hotel_revenue
 *     @brief Associations between ::q08_parsed_arguments_t and it's hotel revenue.
 */
typedef struct {
    GPtrArray  *filter_data;
    GHashTable *hotel_revenue;
} q08_foreach_reservation_data_t;

/**
 * @brief   A method called for each reservation, to calculate the revenue of a hotel.
 * @details An auxiliary method for ::__q08_generate_statistics.
 *
 * @param user_data   A pointer to a ::q08_foreach_reservation_data_t.
 * @param reservation Reservation being processed.
 *
 * @retval 0 Always successful.
 */
int __q08_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q08_foreach_reservation_data_t *foreach_data = (q08_foreach_reservation_data_t *) user_data;

    hotel_id_t hotel_id = reservation_get_hotel_id(reservation);

    for (size_t i = 0; i < foreach_data->filter_data->len; i++) {
        const q08_parsed_arguments_t *args = g_ptr_array_index(foreach_data->filter_data, i);

        if (hotel_id == args->hotel_id) {
            uint16_t price_per_night = reservation_get_price_per_night(reservation);

            date_t reservation_begin = reservation_get_begin_date(reservation),
                   reservation_end   = reservation_get_end_date(reservation);

            /* Reservations don't make money on their last day */
            date_set_day(&reservation_end, date_get_day(reservation_end) - 1);

            if (date_diff(args->begin_date, reservation_end) > 0 ||
                date_diff(reservation_begin, args->end_date) > 0)
                continue;

            date_t range_begin = date_diff(reservation_begin, args->begin_date) < 0
                                     ? args->begin_date
                                     : reservation_begin;
            date_t range_end =
                date_diff(reservation_end, args->end_date) < 0 ? reservation_end : args->end_date;

            uint64_t reservation_revenue =
                price_per_night * (date_diff(range_end, range_begin) + 1);
            uint64_t revenue =
                GPOINTER_TO_UINT(g_hash_table_lookup(foreach_data->hotel_revenue, args));
            g_hash_table_insert(foreach_data->hotel_revenue,
                                (q08_parsed_arguments_t *) args,
                                GUINT_TO_POINTER(revenue + reservation_revenue));
        }
    }

    return 0;
}

/**
 * @brief Generates statistical data for queries of type 8.
 *
 * @param database   Database, to iterate through reservations.
 * @param n          Number of query instances.
 * @param instances  Instances of the query 8.
 *
 * @return A `GHashTable` associating a ::q08_parsed_arguments_t to an integer revenue as a pointer.
 */
void *__q08_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {
    GPtrArray  *filter_data   = g_ptr_array_new();
    GHashTable *hotel_revenue = g_hash_table_new(g_direct_hash, g_direct_equal);

    for (size_t i = 0; i < n; ++i) {
        const q08_parsed_arguments_t *argument_data =
            query_instance_get_argument_data(instances[i]);

        /* TODO - find way to keep const */
        g_hash_table_insert(hotel_revenue, (q08_parsed_arguments_t *) argument_data, 0);
        g_ptr_array_add(filter_data, (q08_parsed_arguments_t *) argument_data);
    }

    q08_foreach_reservation_data_t callback_data = {.filter_data   = filter_data,
                                                    .hotel_revenue = hotel_revenue};

    reservation_manager_iter(database_get_reservations(database),
                             __q08_generate_statistics_foreach_reservation,
                             &callback_data);

    g_ptr_array_unref(filter_data);
    return hotel_revenue;
}

/**
 * @brief Method called to execute a query of type 8.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q08_generate_statistics).
 * @param statistics Statistical data generated by ::__q08_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Always succesl
 * @retval 1 Fatal failure (will only happen if a cosmic ray flips some bit in your memory).
 */
int __q08_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    GHashTable                   *hotel_revenue = (GHashTable *) statistics;
    const q08_parsed_arguments_t *arguments     = query_instance_get_argument_data(instance);

    gpointer revenue_ptr;
    if (g_hash_table_lookup_extended(hotel_revenue, arguments, NULL, &revenue_ptr)) {
        uint64_t revenue = GPOINTER_TO_UINT(revenue_ptr);
        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "revenue", "%" PRIu64, revenue);
    } else {
        fprintf(stderr, "Bad statistical data in query 3! This should not happen!\n");
        return 1;
    }
    return 0;
}

query_type_t *q08_create(void) {
    return query_type_create(8,
                             __q08_parse_arguments,
                             __q08_clone_arguments,
                             free,
                             __q08_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q08_execute);
}
