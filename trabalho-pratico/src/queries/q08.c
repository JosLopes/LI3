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

typedef struct {
    hotel_id_t hotel_code;
    date_t     begin_date;
    date_t     end_date;
} q08_parsed_arguments_t;

void *__q08_parse_arguments(char **argv, size_t argc) {
    if (argc != 3)
        return NULL;

    q08_parsed_arguments_t *parsed_arguments = malloc(sizeof(q08_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    int hotel_retval      = hotel_id_from_string(&parsed_arguments->hotel_code, argv[0]);
    int begin_date_retval = date_from_string(&parsed_arguments->begin_date, argv[1]);
    int end_date_retval   = date_from_string(&parsed_arguments->end_date, argv[2]);

    if (hotel_retval || begin_date_retval || end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }

    return parsed_arguments;
}

typedef struct {
    GPtrArray  *filter_data;
    GHashTable *hotel_revenue;
} q08_foreach_hotel_data_t;

int __q08_generate_statistics_foreach_flight(void *user_data, const reservation_t *reservation) {
    q08_foreach_hotel_data_t *foreach_data = (q08_foreach_hotel_data_t *) user_data;

    hotel_id_t hotel_code = reservation_get_hotel_id(reservation);

    for (size_t i = 0; i < foreach_data->filter_data->len; i++) {
        q08_parsed_arguments_t *args = g_ptr_array_index(foreach_data->filter_data, i);

        /* Check if the hotel meets the hotel_code filter in the arguments */
        if (hotel_code == args->hotel_code) {

            date_t   reservation_begin_date      = reservation_get_begin_date(reservation);
            date_t   reservation_end_date        = reservation_get_end_date(reservation);
            uint16_t reservation_price_per_night = reservation_get_price_per_night(reservation);

            if (date_diff(reservation_begin_date, args->begin_date) >= 0 &&
                date_diff(reservation_end_date, args->end_date) <= 0) {

                uint16_t *revenue =
                    (uint16_t *) g_hash_table_lookup(foreach_data->hotel_revenue, args);
                *revenue += reservation_price_per_night *
                            (date_diff(reservation_end_date, reservation_begin_date) + 1);

            } else if (date_diff(reservation_begin_date, args->begin_date) <= 0 &&
                       date_diff(reservation_end_date, args->end_date) <= 0 &&
                       date_diff(reservation_end_date, args->begin_date) >= 0) {

                uint16_t *revenue =
                    (uint16_t *) g_hash_table_lookup(foreach_data->hotel_revenue, args);
                *revenue += reservation_price_per_night *
                            (date_diff(reservation_end_date, args->begin_date) + 1);

            } else if (date_diff(reservation_begin_date, args->begin_date) >= 0 &&
                       date_diff(reservation_begin_date, args->end_date) <= 0 &&
                       date_diff(reservation_end_date, args->end_date) >= 0) {

                uint16_t *revenue =
                    (uint16_t *) g_hash_table_lookup(foreach_data->hotel_revenue, args);
                *revenue += reservation_price_per_night *
                            (date_diff(args->end_date, reservation_begin_date) + 1);

            } else if (date_diff(reservation_begin_date, args->begin_date) <= 0 &&
                       date_diff(reservation_end_date, args->end_date) >= 0) {

                uint16_t *revenue =
                    (uint16_t *) g_hash_table_lookup(foreach_data->hotel_revenue, args);
                *revenue +=
                    reservation_price_per_night * (date_diff(args->end_date, args->begin_date) + 1);
            }
        }
    }

    return 0;
}

void *__q08_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    GPtrArray  *filter_data   = g_ptr_array_new();
    GHashTable *hotel_revenue = g_hash_table_new(g_direct_hash, g_direct_equal);

    for (size_t i = 0; i < n; ++i) {
        q08_parsed_arguments_t *argument_data = query_instance_get_argument_data(instances);

        uint16_t *revenue = malloc(sizeof(uint16_t));
        *revenue          = 0;

        g_hash_table_insert(hotel_revenue, argument_data, revenue);
        g_ptr_array_add(filter_data, argument_data);
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }

    q08_foreach_hotel_data_t callback_data = {.filter_data   = filter_data,
                                              .hotel_revenue = hotel_revenue};

    reservation_manager_iter(database_get_reservations(database),
                             __q08_generate_statistics_foreach_flight,
                             &callback_data);

    g_ptr_array_unref(filter_data);
    return hotel_revenue;
}

int __q08_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  query_writer_t   *output) {
    (void) database;

    GHashTable             *hotel_revenue = (GHashTable *) statistics;
    q08_parsed_arguments_t *arguments =
        (q08_parsed_arguments_t *) query_instance_get_argument_data(instance);

    uint16_t *revenue = (uint16_t *) g_hash_table_lookup(hotel_revenue, arguments);

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, "revenue", "%d", *(revenue));

    return 0;
}

query_type_t *q08_create(void) {
    return query_type_create(__q08_parse_arguments,
                             free,
                             __q08_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q08_execute);
}
