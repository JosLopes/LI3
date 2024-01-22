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

#include <stdio.h>
#include <stdlib.h>

#include "queries/q08.h"
#include "queries/query_instance.h"

/**
 * @struct q08_parsed_arguments_t
 * @brief  Parsed arguments of a query of type 8.
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
 * @details Asserts that there's three arguments, an hotel identifier and two dates.
 *
 * @param argc Number of arguments.
 * @param argv Values of the arguments.
 *
 * @return `NULL` on parsing or allocation failure, a pointer to a ::q08_parsed_arguments_t
 *         otherwise.
 */
void *__q08_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 3)
        return NULL;

    q08_parsed_arguments_t *const parsed_arguments = malloc(sizeof(q08_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    const int hotel_retval      = hotel_id_from_string(&parsed_arguments->hotel_id, argv[0]);
    const int begin_date_retval = date_from_string(&parsed_arguments->begin_date, argv[1]);
    const int end_date_retval   = date_from_string(&parsed_arguments->end_date, argv[2]);

    if (hotel_retval || begin_date_retval || end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }
    return parsed_arguments;
}

/**
 * @brief  Creates a deep clone of the value returned by ::__q08_parse_arguments.
 * @param  args_data Non-`NULL` value returned by ::__q08_parse_arguments (a pointer to a
 *                   ::q08_parsed_arguments_t).
 * @return A deep copy of @p args_data.
 */
void *__q08_clone_arguments(const void *args_data) {
    const q08_parsed_arguments_t *const args  = args_data;
    q08_parsed_arguments_t *const       clone = malloc(sizeof(q08_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q08_parsed_arguments_t));
    return clone;
}

/**
 * @struct q08_statistical_data_t
 * @brief  Association between query instances and hotel revenue prices.
 *
 * @var q08_statistical_data_t::n
 *     @brief Number of query instances (same as ::q08_statistical_data_t::filters and
 *            ::q08_statistical_data_t::revenues).
 * @var q08_statistical_data_t::filters
 *     @brief Query instances that need to be answered.
 * @var q08_statistical_data_t::revenues
 *     @brief Revenues of hotels (associated to ::q08_statistical_data_t::filters).
 */
typedef struct {
    size_t                  n;
    q08_parsed_arguments_t *filters;
    uint64_t               *revenues;
} q08_statistical_data_t;

/**
 * @brief   A method called for each reservation, to calculate the revenue of a hotel.
 * @details An auxiliary method for ::__q08_generate_statistics.
 *
 * @param user_data   A pointer to a ::q08_statistical_data_t.
 * @param reservation Reservation being processed.
 *
 * @retval 0 Always successful.
 */
int __q08_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q08_statistical_data_t *const stats    = user_data;
    const hotel_id_t              hotel_id = reservation_get_hotel_id(reservation);

    for (size_t i = 0; i < stats->n; i++) {
        const q08_parsed_arguments_t *const args = &stats->filters[i];

        if (hotel_id == args->hotel_id) {
            const uint16_t price_per_night   = reservation_get_price_per_night(reservation);
            const date_t   reservation_begin = reservation_get_begin_date(reservation);
            date_t         reservation_end   = reservation_get_end_date(reservation);

            /* Reservations don't make money on their last day */
            date_set_day(&reservation_end, date_get_day(reservation_end) - 1);
            if (date_diff(args->begin_date, reservation_end) > 0 ||
                date_diff(reservation_begin, args->end_date) > 0)
                continue;

            const date_t range_begin = date_diff(reservation_begin, args->begin_date) < 0
                                           ? args->begin_date
                                           : reservation_begin;
            const date_t range_end =
                date_diff(reservation_end, args->end_date) < 0 ? reservation_end : args->end_date;

            stats->revenues[i] += price_per_night * (date_diff(range_end, range_begin) + 1);
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
 * @return A pointer to a ::q08_statistical_data_t on success, or `NULL` on allocation failure.
 */
void *__q08_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {

    q08_statistical_data_t *const stats = malloc(sizeof(q08_statistical_data_t));
    if (!stats)
        return NULL;
    stats->n = n;

    stats->filters = malloc(n * sizeof(q08_parsed_arguments_t));
    if (!stats->filters) {
        free(stats);
        return NULL;
    }

    stats->revenues = malloc(n * sizeof(uint64_t));
    if (!stats->revenues) {
        free(stats->filters);
        free(stats);
        return NULL;
    }

    for (size_t i = 0; i < n; ++i) {
        const q08_parsed_arguments_t *const argument_data =
            query_instance_get_argument_data(instances[i]);

        stats->filters[i]  = *argument_data;
        stats->revenues[i] = 0;
    }

    reservation_manager_iter(database_get_reservations(database),
                             __q08_generate_statistics_foreach_reservation,
                             stats);
    return stats;
}

/**
 * @brief Frees statistical data generated by ::__q08_generate_statistics.
 * @param statistical_data Non-`NULL` value returned by ::__q08_generate_statistics.
 */
void __q08_free_statistics(void *statistical_data) {
    q08_statistical_data_t *const stats = statistical_data;
    free(stats->revenues);
    free(stats->filters);
    free(stats);
}

/**
 * @brief Method called to execute a query of type 8.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q08_generate_statistics).
 * @param statistics Statistical data generated by ::__q08_generate_statistics (a pointer to a
 *                   ::q08_statistical_data_t).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success.
 * @retval 1 Fatal failure (should, in principle, be unreachable).
 */
int __q08_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q08_statistical_data_t *const stats     = statistics;
    const q08_parsed_arguments_t *const arguments = query_instance_get_argument_data(instance);

    for (size_t i = 0; i < stats->n; ++i) {
        if (stats->filters[i].hotel_id == arguments->hotel_id &&
            stats->filters[i].begin_date == arguments->begin_date &&
            stats->filters[i].end_date == arguments->end_date) {

            query_writer_write_new_object(output);
            query_writer_write_new_field(output, "revenue", "%" PRIu64, stats->revenues[i]);
            return 0;
        }
    }

    fprintf(stderr, "Bad statistical data in query 8! This should not happen!\n");
    return 1;
}

query_type_t *q08_create(void) {
    return query_type_create(8,
                             __q08_parse_arguments,
                             __q08_clone_arguments,
                             free,
                             __q08_generate_statistics,
                             __q08_free_statistics,
                             __q08_execute);
}
