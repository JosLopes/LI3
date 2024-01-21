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

#include <glib.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

#include "queries/q10.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

/**
 * @struct q10_parsed_arguments_t
 * @brief  A structure to hold the parsed arguments for a query of type 10.
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
 * @brief   Parses arguments of a query of type 10.
 * @details Asserts there are 0 to 2 arguments, a year and a month, respectively.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` for invalid arguments, a copy of the only @p argv on success.
 */
void *__q10_parse_arguments(char *const *argv, size_t argc) {
    q10_parsed_arguments_t *ret = malloc(sizeof(q10_parsed_arguments_t));
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

#define STARTING_YEAR 1989
#define LAST_YEAR     2052

uint16_t __q10_key_gen(const date_t date) {
    return ((date_get_year(date) - STARTING_YEAR) << 9) | (date_get_month(date) << 5) |
           (date_get_day(date));
}

/**
 * @brief  Creates a deep copy of a ::q10_parsed_arguments_t.
 * @param  args_data Value returned by ::__q10_parse_arguments.
 * @return A deep clone of @p args_data
 */
void *__q10_clone_arguments(const void *args_data) {
    const q10_parsed_arguments_t *args  = args_data;
    q10_parsed_arguments_t       *clone = malloc(sizeof(q10_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q10_parsed_arguments_t));
    return clone;
}

/**
 * @brief Statistical information about the dataset in a given day, year or month.
 *
 * @var q10_instant_statistics::users
 *     @brief Number of registered users.
 * @var q10_instant_statistics::flights
 *     @brief Number of flights.
 * @var q10_instant_statistics::passengers
 *     @brief Number of passengers.
 * @var q10_instant_statistics::unique_passengers
 *     @brief Number of passengers with only one flight in this day / month / year.
 * @var q10_instant_statistics::reservation
 *     @brief Number of hotel reservations.
 * @var q10_instant_statistics::date
 *     @brief Instant (day / month / year) of this instant.
 */
typedef struct {
    uint64_t users, flights, passengers, unique_passengers, reservations;
} q10_instant_statistics_t;

/**
 * @brief Type of `user_data` parameter in ::__q10_generate_statistics_foreach_user.
 *
 * @var q10_foreach_user_data_t::stats
 *     @brief Statistical data to be modified and returned by ::__q10_generate_statistics.
 * @var q10_foreach_user_data_t::flight
 *     @brief Flight manager to access flight informayeartion.
 * @var q10_foreach_user_data_t::aux_counted
 *     @brief Auxiliary hash table that is stripped of all keys for each user (avoids reallocations).
 */
typedef struct {
    q10_instant_statistics_t **stats;
    const flight_manager_t    *flights;
    int                       *aux_counted;
} q10_foreach_user_data_t;

#define STATS_ARRAY_LENGTH pow(2, 15)

/**
 * @brief Method called for each user, to generate statistical data.
 * @details An auxiliary method for ::__q10_generate_statistics.
 *
 * @param user_data A `GHashTable` that associates dates (also dayless and monthless dates) to
 *                  pointers to ::q10_instant_statistics_t.
 * @param user      The user to consider.
 *
 * @retval 0 Success
 * @retval 1 Allocation error
 */
int __q10_generate_statistics_foreach_user(void                               *user_data,
                                           const user_t                       *user,
                                           const single_pool_id_linked_list_t *passengers) {
    q10_foreach_user_data_t *iter_data = user_data;

    date_t   date = user_get_account_creation_date(user);
    uint16_t key  = __q10_key_gen(date);

    if ((iter_data->stats)[key])
        ((iter_data->stats)[key])->users++;

    memset(iter_data->aux_counted, 0, STATS_ARRAY_LENGTH * sizeof(int));
    while (passengers) {
        const flight_t *flight =
            flight_manager_get_by_id(iter_data->flights,
                                     single_pool_id_linked_list_get_value(passengers));
        date = date_and_time_get_date(flight_get_schedule_departure_date(flight));

        uint16_t monthless_key = __q10_key_gen(date_generate_monthless(date));
        if ((iter_data->stats)[monthless_key]) {
            if (!(iter_data->aux_counted)[monthless_key]) {
                ((iter_data->stats)[monthless_key])->unique_passengers++;
                (iter_data->aux_counted)[monthless_key] = 1;
            }
        }

        uint16_t dayless_key = __q10_key_gen(date_generate_dayless(date));
        if ((iter_data->stats)[dayless_key]) {
            if (!(iter_data->aux_counted)[dayless_key]) {
                ((iter_data->stats)[dayless_key])->unique_passengers++;
                (iter_data->aux_counted)[dayless_key] = 1;
            }
        }

        uint16_t passengers_key = __q10_key_gen(date);
        if ((iter_data->stats)[passengers_key]) {
            if (!(iter_data->aux_counted)[passengers_key]) {
                ((iter_data->stats)[passengers_key])->unique_passengers++;
                (iter_data->aux_counted)[passengers_key] = 1;
            }

            ((iter_data->stats)[passengers_key])->passengers++;
        }

        passengers = single_pool_id_linked_list_get_next(passengers);
    }

    return 0;
}

/**
 * @brief Method called for each flight, to generate statistical data.
 * @details An auxiliary method for ::__q10_generate_statistics.
 *
 * @param user_data A `GHashTable` that associates dates (also dayless and monthless dates) to
 *                  pointers to ::q10_instant_statistics_t.
 * @param flight    The flight to consider.
 *
 * @retval 0 Success
 * @retval 1 Allocation error
 */
int __q10_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    q10_instant_statistics_t **stats = (q10_instant_statistics_t **) user_data;

    date_t   date = date_and_time_get_date(flight_get_schedule_departure_date(flight));
    uint16_t key  = __q10_key_gen(date);

    if (stats[key])
        (stats[key])->flights++;

    return 0;
}

/**
 * @brief Method called for each reservation, to generate statistical data.
 * @details An auxiliary method for ::__q10_generate_statistics.
 *
 * @param user_data   A `GHashTable` that associates dates (also dayless and monthless dates) to
 *                    pointers to ::q10_instant_statistics_t.
 * @param reservation The reservation to consider.
 *
 * @retval 0 Success
 * @retval 1 Allocation error
 */
int __q10_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q10_instant_statistics_t **stats = (q10_instant_statistics_t **) user_data;

    date_t   date = reservation_get_begin_date(reservation);
    uint16_t key  = __q10_key_gen(date);

    if (stats[key])
        (stats[key])->reservations++;

    return 0;
}

/**
 * @brief Generates statistical data for queries of type 10.
 *
 * @param database  Database, to iterate through users.
 * @param instances Query instances that will need to be executed.
 * @param n         Number of query instances that will need to be executed.
 *
 * @return A `GHashTable` that associates dates (also dayless and monthless dates) to pointers to
 *         ::q10_instant_statistics_t (`NULL` on failure).
 */
void *__q10_generate_statistics(const database_t              *database,
                                const query_instance_t *const *instances,
                                size_t                         n) {

    q10_instant_statistics_t **stats =
        malloc(STATS_ARRAY_LENGTH * sizeof(q10_instant_statistics_t *));
    if (!stats)
        return NULL;

    /* This is not very portable! */
    memset(stats, 0, STATS_ARRAY_LENGTH * sizeof(q10_instant_statistics_t *));
    for (size_t i = 0; i < n; ++i) {
        const q10_parsed_arguments_t *args = query_instance_get_argument_data(instances[i]);

        if (args->year == -1 && args->month == -1) {
            date_t date;

            for (int year = STARTING_YEAR; year <= LAST_YEAR; ++year) {
                for (int month = 1; month <= 12; ++month) {
                    for (int day = 1; day <= 31; ++day) {
                        date_from_values(&date, year, month, day);
                        uint16_t key = __q10_key_gen(date);

                        if (!stats[key]) {
                            q10_instant_statistics_t *istats =
                                malloc(sizeof(q10_instant_statistics_t));
                            if (!istats) {
                                free(stats);
                                return NULL;
                            }

                            memset(istats, 0, sizeof(q10_instant_statistics_t));
                            stats[key] = istats;
                        } else {
                            continue;
                        }
                    }
                }

                date         = date_generate_monthless(date);
                uint16_t key = __q10_key_gen(date);

                if (!stats[key]) {
                    q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                    if (!istats) {
                        free(stats);
                        return NULL;
                    }

                    memset(istats, 0, sizeof(q10_instant_statistics_t));
                    stats[key] = istats;
                }
            }
        } else if (args->year != -1 && args->month == -1) {
            date_t date;

            for (int month = 1; month <= 12; ++month) {
                for (int day = 1; day <= 31; ++day) {
                    date_from_values(&date, args->year, month, day);
                    uint16_t key = __q10_key_gen(date);

                    if (!stats[key]) {
                        q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                        if (!istats) {
                            free(stats);
                            return NULL;
                        }

                        memset(istats, 0, sizeof(q10_instant_statistics_t));
                        stats[key] = istats;
                    } else {
                        continue;
                    }
                }

                date         = date_generate_dayless(date);
                uint16_t key = __q10_key_gen(date);

                if (!stats[key]) {
                    q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                    if (!istats) {
                        free(stats);
                        return NULL;
                    }

                    memset(istats, 0, sizeof(q10_instant_statistics_t));
                    stats[key] = istats;
                }
            }
        } else if (args->month != -1) {
            date_t date;

            for (int day = 1; day <= 31; ++day) {
                date_from_values(&date, args->year, args->month, day);
                uint16_t key = __q10_key_gen(date);

                if (!stats[key]) {
                    q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                    if (!istats) {
                        free(stats);
                        return NULL;
                    }

                    memset(istats, 0, sizeof(q10_instant_statistics_t));
                    stats[key] = istats;
                } else {
                    continue;
                }
            }
        }
    }

    q10_foreach_user_data_t user_iter_data = {.stats   = stats,
                                              .flights = database_get_flights(database),
                                              .aux_counted =
                                                  malloc(STATS_ARRAY_LENGTH * sizeof(int))};
    user_manager_iter_with_flights(database_get_users(database),
                                   __q10_generate_statistics_foreach_user,
                                   &user_iter_data);
    free(user_iter_data.aux_counted);

    flight_manager_iter(database_get_flights(database),
                        __q10_generate_statistics_foreach_flight,
                        stats);

    reservation_manager_iter(database_get_reservations(database),
                             __q10_generate_statistics_foreach_reservation,
                             stats);

    return stats;
}

/**
 * @brief Writes the information of a ::q10_instant_statistics_t to a ::query_writer_t.
 *
 * @param istats Information to be printed
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
    query_writer_write_new_field(output, "users", "%" PRIu64, istats->users);
    query_writer_write_new_field(output, "flights", "%" PRIu64, istats->flights);
    query_writer_write_new_field(output, "passengers", "%" PRIu64, istats->passengers);
    query_writer_write_new_field(output,
                                 "unique_passengers",
                                 "%" PRIu64,
                                 istats->unique_passengers);
    query_writer_write_new_field(output, "reservations", "%" PRIu64, istats->reservations);
}

void __q10_add_instants(q10_instant_statistics_t *a, q10_instant_statistics_t *b) {
    b->users += a->users;
    b->flights += a->flights;
    b->passengers += a->passengers;
    b->reservations += a->reservations;
}

int __q10_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q10_parsed_arguments_t *args  = query_instance_get_argument_data(instance);
    q10_instant_statistics_t    **stats = (q10_instant_statistics_t **) statistics;

    if (args->year == -1) {
        date_t                   date;
        q10_instant_statistics_t results;

        for (int year = STARTING_YEAR; year <= LAST_YEAR; ++year) {
            memset(&results, 0, sizeof(q10_instant_statistics_t));

            date_from_values(&date, year, 1, 1);
            date         = date_generate_monthless(date);
            uint16_t key = __q10_key_gen(date);
            if (stats[key])
                results.unique_passengers = (stats[key])->unique_passengers;
            else
                continue;

            for (int month = 1; month <= 12; ++month) {
                for (int day = 1; day <= 31; ++day) {
                    date_from_values(&date, year, month, day);
                    uint16_t key = __q10_key_gen(date);

                    __q10_add_instants(stats[key], &results);
                }
            }

            if (results.users || results.flights || results.passengers ||
                results.unique_passengers || results.reservations)
                __q10_instant_statistics_write(&results, output, "year", year);
        }
    } else if (args->month == -1) {
        date_t                   date;
        q10_instant_statistics_t results;

        for (int month = 1; month <= 12; ++month) {
            memset(&results, 0, sizeof(q10_instant_statistics_t));

            date_from_values(&date, args->year, month, 1);
            date         = date_generate_dayless(date);
            uint64_t key = __q10_key_gen(date);
            if (stats[key])
                results.unique_passengers = (stats[key])->unique_passengers;
            else
                continue;

            for (int day = 1; day <= 31; ++day) {
                date_from_values(&date, args->year, month, day);
                uint16_t key = __q10_key_gen(date);

                __q10_add_instants(stats[key], &results);
            }

            if (results.users || results.flights || results.passengers ||
                results.unique_passengers || results.reservations)
                __q10_instant_statistics_write(&results, output, "month", month);
        }
    } else if (args->month != -1) {
        date_t date;

        for (int day = 1; day <= 31; ++day) {
            date_from_values(&date, args->year, args->month, day);
            uint16_t key = __q10_key_gen(date);

            q10_instant_statistics_t *istats = stats[key];
            if (istats && (istats->users || istats->flights || istats->passengers ||
                           istats->unique_passengers || istats->reservations))
                __q10_instant_statistics_write(istats, output, "day", day);
        }
    }

    return 0;
}

void __q10_free_statistics(void *statistics) {
    q10_instant_statistics_t **stats = (q10_instant_statistics_t **) statistics;

    for (size_t i = 0; i < STATS_ARRAY_LENGTH; ++i)
        if (stats[i])
            free(stats[i]);

    free(stats);
}

query_type_t *q10_create(void) {
    return query_type_create(__q10_parse_arguments,
                             __q10_clone_arguments,
                             free,
                             __q10_generate_statistics,
                             __q10_free_statistics,
                             __q10_execute);
}
