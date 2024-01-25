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
 * @file  q10.c
 * @brief Implementation of methods in include/queries/q10.h
 */

/* TODO - remove headers */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/q10.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @struct q10_parsed_arguments_t
 * @brief  Parsed arguments for a query of type 10.
 *
 * @var q10_parsed_arguments_t::year
 *     @brief The year to consider (`-1` if not specified).
 * @var q10_parsed_arguments_t::month
 *     @brief The month to consider (`-1` if not specified).
 */
typedef struct {
    int16_t year;
    int8_t  month;
} q10_parsed_arguments_t;

/**
 * @brief   Parses the arguments for a query of type 10.
 * @details There can be zero to two arguments, a year and a month.
 *
 * @param argc Number of query arguments.
 * @param argv List of query arguments.
 *
 * @return A pointer to a ::q10_parsed_arguments_t, or `NULL` on parsing or allocation failure.
 */
void *__q10_parse_arguments(size_t argc, char *const argv[argc]) {
    q10_parsed_arguments_t *const ret = malloc(sizeof(q10_parsed_arguments_t));
    if (!ret)
        return NULL;

    if (argc == 0) {
        ret->year  = -1;
        ret->month = -1;
    } else if (argc == 1) {
        uint64_t parsed;
        if (int_utils_parse_positive(&parsed, argv[0])) {
            free(ret);
            return NULL;
        }

        ret->year  = parsed;
        ret->month = -1;
    } else if (argc == 2) {
        uint64_t parsed_year, parsed_month;
        if (int_utils_parse_positive(&parsed_year, argv[0]) ||
            int_utils_parse_positive(&parsed_month, argv[1])) {

            free(ret);
            return NULL;
        }

        if (0 == parsed_month || parsed_month > 12) {
            free(ret);
            return NULL;
        }

        ret->year  = parsed_year;
        ret->month = parsed_month;
    } else {
        free(ret);
        return NULL;
    }

    return ret;
}

/**
 * @brief  Creates a deep clone of the value returned by ::__q10_parse_arguments.
 * @param  args_data Non-`NULL` value returned by ::__q10_parse_arguments (a pointer to a
 *                   ::q10_parsed_arguments_t).
 * @return A deep copy of @p args_data.
 */
void *__q10_clone_arguments(const void *args_data) {
    const q10_parsed_arguments_t *const args  = args_data;
    q10_parsed_arguments_t *const       clone = malloc(sizeof(q10_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q10_parsed_arguments_t));
    return clone;
}

/**
 * @struct q10_instant_statistics_t
 * @brief  Statistical information about the dataset in a given day, year or month.
 *
 * @var q10_instant_statistics_t::users
 *     @brief Number of registered users.
 * @var q10_instant_statistics_t::flights
 *     @brief Number of flights.
 * @var q10_instant_statistics_t::passengers
 *     @brief Number of passengers.
 * @var q10_instant_statistics_t::unique_passengers
 *     @brief Number of passengers with only one flight in this day / month / year.
 * @var q10_instant_statistics_t::reservations
 *     @brief Number of hotel reservations.
 */
typedef struct {
    uint32_t users, flights, passengers, unique_passengers, reservations;
} q10_instant_statistics_t;

/**
 * @brief   A type of match between an event and query arguments.
 * @details Return type of ::__q10_instant_matches.
 */
typedef enum {
    Q10_INSTANT_MATCHES_NO_MATCH,    /**< Event isn't fit for the arguments. */
    Q10_INSTANT_MATCHES_ALL_YEARS,   /**< Event matches and must be added to an array of years. */
    Q10_INSTANT_MATCHES_YEAR_MONTHS, /**< Event matches and must be added to an array of months. */
    Q10_INSTANT_MATCHES_MONTH_DAYS,  /**< Event matches and must be added to an array of days. */
} q10_instant_matches_ret_t;

/**
 * @brief Determines if an event matches the arguments of a query, and should be counted towards its
 *        results.
 *
 * @param year   Year of the event.
 * @param month  Month of the event.
 * @param filter Arguments to match event against.
 *
 * @return The type of match (see ::q10_instant_matches_ret_t).
 */
__attribute__((always_inline)) inline q10_instant_matches_ret_t
    __q10_instant_matches(int16_t year, int8_t month, const q10_parsed_arguments_t *filter) {
    if (year == filter->year) {
        const q10_instant_matches_ret_t alternative =
            (filter->month == -1) ? Q10_INSTANT_MATCHES_YEAR_MONTHS : Q10_INSTANT_MATCHES_NO_MATCH;
        return (month == filter->month) ? Q10_INSTANT_MATCHES_MONTH_DAYS : alternative;
    }

    return (filter->year == -1) ? Q10_INSTANT_MATCHES_ALL_YEARS : Q10_INSTANT_MATCHES_NO_MATCH;
}

/**
 * @struct q10_foreach_user_data_t
 * @brief  Data used while iterating through users (this includes passengers relations).
 *
 * @var q10_foreach_user_data_t::stats_data
 *     @brief   Statistics (array of arrays) for the moments related to query arguments.
 *     @details The length of each array depends on the arguments in each element of
 *              ::q10_foreach_user_data_t::filters.
 * @var q10_foreach_user_data_t::filters
 *     @brief Array of query arguments to match events against.
 * @var q10_foreach_user_data_t::flags
 *     @brief Array of auxiliary bit arrays (integers), used to calculate
 *            ::q10_instant_statistics_t::unique_passengers
 * @var q10_foreach_user_data_t::stats_length
 *     @brief Length of ::q10_foreach_user_data_t::stats_data, ::q10_foreach_user_data_t::filters
 *            and q10_foreach_user_data_t::flags.
 * @var q10_foreach_user_data_t::flights
 *     @brief Flight manager for performant access to flights.
 */
typedef struct {
    q10_instant_statistics_t *const *const stats_data;
    const q10_parsed_arguments_t *const    filters;
    uint64_t *const                        flags;
    const size_t                           stats_length;

    const flight_manager_t *const flights;
} q10_foreach_user_data_t;

/** @brief Start of the range of event years supported by this query. */
#define Q10_SUPPORTED_YEAR_RANGE_START 2000
/** @brief End of the range of event years supported by this query. */
#define Q10_SUPPORTED_YEAR_RANGE_END   2064
/** @brief The number event of years supported by this query. */
#define Q10_SUPPORTED_YEAR_RANGE_AMPLITUDE                                                         \
    (Q10_SUPPORTED_YEAR_RANGE_END - Q10_SUPPORTED_YEAR_RANGE_START)

/**
 * @brief   Method called for every user in the database.
 * @details Calculates the number of users, passengers, and unique passengers for every query.
 *
 * @param user_data  A pointer to a ::q10_foreach_user_data_t.
 * @param user       User being processed.
 * @param passengers Identifiers of the flights @p user has been in.
 *
 * @retval 0 Always, not to stop iteration.
 */
int __q10_generate_statistics_foreach_user(void                               *user_data,
                                           const user_t                       *user,
                                           const single_pool_id_linked_list_t *passengers) {
    q10_foreach_user_data_t *const iter_data = user_data;

    date_t  date  = date_and_time_get_date(user_get_account_creation_date(user));
    int16_t year  = date_get_year(date);
    uint8_t month = date_get_month(date);
    uint8_t day   = date_get_day(date);

    for (size_t i = 0; i < iter_data->stats_length; ++i) {
        switch (__q10_instant_matches(year, month, iter_data->filters + i)) {
            case Q10_INSTANT_MATCHES_NO_MATCH:
                break;
            case Q10_INSTANT_MATCHES_ALL_YEARS:
                iter_data->stats_data[i][year - Q10_SUPPORTED_YEAR_RANGE_START].users++;
                break;
            case Q10_INSTANT_MATCHES_YEAR_MONTHS:
                iter_data->stats_data[i][month].users++;
                break;
            case Q10_INSTANT_MATCHES_MONTH_DAYS:
                iter_data->stats_data[i][day].users++;
                break;
        }

        /* Reset flags for unique passengers calculation. */
        iter_data->flags[i] = 0;
    }

    while (passengers) {
        const flight_t *const flight =
            flight_manager_get_by_id(iter_data->flights,
                                     single_pool_id_linked_list_get_value(passengers));
        passengers = single_pool_id_linked_list_get_next(passengers);
        __builtin_prefetch(passengers); /* <- This does miracles for performance */

        date  = date_and_time_get_date(flight_get_schedule_departure_date(flight));
        year  = date_get_year(date);
        month = date_get_month(date);
        day   = date_get_day(date);

        for (size_t i = 0; i < iter_data->stats_length; ++i) {
            q10_instant_statistics_t *target;
            int                       shift, can_add_unique;
            switch (__q10_instant_matches(year, month, iter_data->filters + i)) {
                case Q10_INSTANT_MATCHES_NO_MATCH:
                    continue;
                case Q10_INSTANT_MATCHES_ALL_YEARS:
                    target = &iter_data->stats_data[i][year - Q10_SUPPORTED_YEAR_RANGE_START];

                    shift          = 1 << (year - Q10_SUPPORTED_YEAR_RANGE_START);
                    can_add_unique = !(iter_data->flags[i] & shift);
                    iter_data->flags[i] |= shift;
                    break;
                case Q10_INSTANT_MATCHES_YEAR_MONTHS:
                    target = &iter_data->stats_data[i][month];

                    shift          = 1 << month;
                    can_add_unique = !(iter_data->flags[i] & shift);
                    iter_data->flags[i] |= shift;
                    break;
                case Q10_INSTANT_MATCHES_MONTH_DAYS:
                    target = &iter_data->stats_data[i][day];

                    shift          = 1 << day;
                    can_add_unique = !(iter_data->flags[i] & shift);
                    iter_data->flags[i] |= shift;
                    break;
            }

            target->passengers++;
            target->unique_passengers += can_add_unique;
        }
    }
    return 0;
}

/**
 * @struct q10_statistical_data_t
 * @brief  Answers to all queries of type 10.
 *
 * @var q10_statistical_data_t::stats_data
 *     @brief   Statistics (array of arrays) for the moments related to query arguments.
 *     @details The length of each array depends on the arguments in each element of
 *              ::q10_statistical_data_t::filters.
 * @var q10_statistical_data_t::filters
 *     @brief Array of query arguments to match events against.
 * @var q10_statistical_data_t::stats_length
 *     @brief Length of ::q10_statistical_data_t::stats_data and ::q10_statistical_data_t::filters.
 */
typedef struct {
    q10_instant_statistics_t **stats_data;
    q10_parsed_arguments_t    *filters;
    size_t                     stats_length;
} q10_statistical_data_t;

/**
 * @brief Method called for every flight in the database.
 *
 * @param user_data  A pointer to a ::q10_statistical_data_t.
 * @param flight     Flight being processed.
 *
 * @retval 0 Always, not to stop iteration.
 */
int __q10_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    q10_statistical_data_t *const stats = user_data;

    const date_t   date  = date_and_time_get_date(flight_get_schedule_departure_date(flight));
    const uint16_t year  = date_get_year(date);
    const uint8_t  month = date_get_month(date);
    const uint8_t  day   = date_get_day(date);

    for (size_t i = 0; i < stats->stats_length; ++i) {
        switch (__q10_instant_matches(year, month, stats->filters + i)) {
            case Q10_INSTANT_MATCHES_NO_MATCH:
                break;
            case Q10_INSTANT_MATCHES_ALL_YEARS:
                stats->stats_data[i][year - Q10_SUPPORTED_YEAR_RANGE_START].flights++;
                break;
            case Q10_INSTANT_MATCHES_YEAR_MONTHS:
                stats->stats_data[i][month].flights++;
                break;
            case Q10_INSTANT_MATCHES_MONTH_DAYS:
                stats->stats_data[i][day].flights++;
                break;
        }
    }

    return 0;
}

/**
 * @brief Method called for every reservation in the database.
 *
 * @param user_data   A pointer to a ::q10_statistical_data_t.
 * @param reservation Reservation being processed.
 *
 * @retval 0 Always, not to stop iteration.
 */
int __q10_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q10_statistical_data_t *const stats = user_data;

    const date_t   date  = reservation_get_begin_date(reservation);
    const uint16_t year  = date_get_year(date);
    const uint8_t  month = date_get_month(date);
    const uint8_t  day   = date_get_day(date);

    for (size_t i = 0; i < stats->stats_length; ++i) {
        switch (__q10_instant_matches(year, month, stats->filters + i)) {
            case Q10_INSTANT_MATCHES_NO_MATCH:
                break;
            case Q10_INSTANT_MATCHES_ALL_YEARS:
                stats->stats_data[i][year - Q10_SUPPORTED_YEAR_RANGE_START].reservations++;
                break;
            case Q10_INSTANT_MATCHES_YEAR_MONTHS:
                stats->stats_data[i][month].reservations++;
                break;
            case Q10_INSTANT_MATCHES_MONTH_DAYS:
                stats->stats_data[i][day].reservations++;
                break;
        }
    }

    return 0;
}

/**
 * @brief Generates statistical data for queries of type 10.
 *
 * @param database   Database to iterate through.
 * @param n          Number of query instances.
 * @param instances  Instances of the query 10.
 *
 * @return A pointer to a ::q10_statistical_data_t on success, or `NULL` on allocation failure.
 */
void *__q10_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {

    q10_instant_statistics_t **const stats_data = calloc(n, sizeof(q10_instant_statistics_t *));
    if (!stats_data)
        goto DEFER_1;

    q10_parsed_arguments_t *const filters = malloc(n * sizeof(q10_parsed_arguments_t));
    if (!filters)
        goto DEFER_2;

    uint64_t *const flags = malloc(n * sizeof(uint64_t));
    if (!flags)
        goto DEFER_3;

    for (size_t i = 0; i < n; ++i) {
        const q10_parsed_arguments_t *const args = query_instance_get_argument_data(instances[i]);
        filters[i]                               = *args;

        if (args->year != -1 && args->month != -1) {
            stats_data[i] = calloc(32, sizeof(q10_instant_statistics_t));
            if (!stats_data[i])
                goto DEFER_4;
        } else if (args->year != -1) {
            stats_data[i] = calloc(13, sizeof(q10_instant_statistics_t));
            if (!stats_data[i])
                goto DEFER_4;
        } else {
            stats_data[i] =
                calloc(Q10_SUPPORTED_YEAR_RANGE_AMPLITUDE, sizeof(q10_instant_statistics_t));
            if (!stats_data[i])
                goto DEFER_4;
        }
    }

    q10_foreach_user_data_t user_iter_data = {.stats_data   = stats_data,
                                              .filters      = filters,
                                              .flags        = flags,
                                              .stats_length = n,
                                              .flights      = database_get_flights(database)};
    user_manager_iter_with_flights(database_get_users(database),
                                   __q10_generate_statistics_foreach_user,
                                   &user_iter_data);

    q10_statistical_data_t *const stats = malloc(sizeof(q10_statistical_data_t));
    if (!stats)
        goto DEFER_4;
    stats->stats_data   = stats_data;
    stats->filters      = filters;
    stats->stats_length = n;

    flight_manager_iter(database_get_flights(database),
                        __q10_generate_statistics_foreach_flight,
                        stats);

    reservation_manager_iter(database_get_reservations(database),
                             __q10_generate_statistics_foreach_reservation,
                             stats);

    free(flags);
    return stats;

DEFER_4:
    for (size_t i = 0; i < n; ++i)
        if (stats_data[i])
            free(stats_data[i]);
    free(flags);
DEFER_3:
    free(filters);
DEFER_2:
    free(stats_data);
DEFER_1:
    return NULL;
}

/**
 * @brief Writes the information of a ::q10_instant_statistics_t to a ::query_writer_t.
 *
 * @param istats Information to be printed.
 * @param output Where to output the data in @p istats to.
 * @param ymd    Type of instant (`"year"`, `"month"` or `"date"`).
 * @param value  Value of the instant that @p istats refers to (year, month or day).
 */
void __q10_instant_statistics_write(const q10_instant_statistics_t *istats,
                                    query_writer_t                 *output,
                                    const char                     *ymd,
                                    int                             value) {

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, ymd, "%d", value);
    query_writer_write_new_field(output, "users", "%" PRIu32, istats->users);
    query_writer_write_new_field(output, "flights", "%" PRIu32, istats->flights);
    query_writer_write_new_field(output, "passengers", "%" PRIu32, istats->passengers);
    query_writer_write_new_field(output,
                                 "unique_passengers",
                                 "%" PRIu32,
                                 istats->unique_passengers);
    query_writer_write_new_field(output, "reservations", "%" PRIu32, istats->reservations);
}

/**
 * @brief   Checks if a day, month or year has events happening in it.
 * @details If it hasn't, then it shouldn't be printed.
 *
 * @param istats Event count in a day / month / year.
 *
 * @return Whether there is at least one event in @p istats.
 */
int __q10_instant_statistics_has_events(const q10_instant_statistics_t *istats) {
    return istats->users || istats->flights || istats->passengers || istats->unique_passengers ||
           istats->reservations;
}

/**
 * @brief Method called to execute a query of type 10.
 *
 * @param database   Database to get data from (not used, as all data is collected in
 *                   ::__q10_generate_statistics).
 * @param statistics Statistical data generated by ::__q10_generate_statistics (a pointer to a
 *                   ::q10_statistical_data_t).
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's result to.
 *
 * @retval 0 Success.
 * @retval 1 Fatal failure (should, in principle, be unreachable).
 */
int __q10_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q10_parsed_arguments_t *const args  = query_instance_get_argument_data(instance);
    const q10_statistical_data_t *const stats = statistics;

    for (size_t i = 0; i < stats->stats_length; ++i) {
        const q10_parsed_arguments_t *const filter = stats->filters + i;
        if (!(filter->year == args->year && filter->month == args->month))
            continue;

        const q10_instant_statistics_t *const istats = stats->stats_data[i];

        if (filter->year != -1 && filter->month != -1) {
            for (int j = 1; j <= 31; ++j)
                if (__q10_instant_statistics_has_events(istats + j))
                    __q10_instant_statistics_write(istats + j, output, "day", j);
        } else if (filter->year != -1) {
            for (int j = 1; j <= 12; ++j)
                if (__q10_instant_statistics_has_events(istats + j))
                    __q10_instant_statistics_write(istats + j, output, "month", j);
        } else {
            for (int j = 0; j < Q10_SUPPORTED_YEAR_RANGE_AMPLITUDE; ++j)
                if (__q10_instant_statistics_has_events(istats + j))
                    __q10_instant_statistics_write(istats + j,
                                                   output,
                                                   "year",
                                                   j + Q10_SUPPORTED_YEAR_RANGE_START);
        }
        return 0;
    }

    fprintf(stderr, "Bad statistical data in query 10! This should not happen!\n");
    return 1;
}

/**
 * @brief Frees statistical data generated by ::__q10_generate_statistics.
 * @param statistics Non-`NULL` value returned by ::__q10_generate_statistics.
 */
void __q10_free_statistics(void *statistics) {
    q10_statistical_data_t *const stats = statistics;

    for (size_t i = 0; i < stats->stats_length; ++i)
        free((stats->stats_data)[i]);
    free(stats->stats_data);
    free(stats->filters);
    free(stats);
}

query_type_t *q10_create(void) {
    return query_type_create(10,
                             __q10_parse_arguments,
                             __q10_clone_arguments,
                             free,
                             __q10_generate_statistics,
                             __q10_free_statistics,
                             __q10_execute);
}
