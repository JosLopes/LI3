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
 */
typedef struct {
    uint64_t users, flights, passengers, unique_passengers, reservations;
} q10_instant_statistics_t;

/**
 * @brief Fills @p instants with statistics that need to be modified if an event hapenned in @p
 *        date.
 *
 * @param stats    Query 10 statistics (dates -> q10_instant_statistics_t).
 * @param instants Will be filled with pointer to ::q10_instant_statistics_t and `NULL`s.
 * @param date     Date when an event hapenned.
 *
 * @retval 0 Success
 * @retval 1 Allocation error
 */
int __q10_fill_instants(GHashTable *stats, q10_instant_statistics_t *instants[3], date_t date) {
    instants[0] = g_hash_table_lookup(stats, GUINT_TO_POINTER(date));
    instants[1] = g_hash_table_lookup(stats, GUINT_TO_POINTER(date_generate_dayless(date)));

    gpointer                  monthless = GUINT_TO_POINTER(date_generate_monthless(date));
    q10_instant_statistics_t *year      = g_hash_table_lookup(stats, monthless);
    if (!year) {
        year = malloc(sizeof(q10_instant_statistics_t));
        if (!year)
            return 1;
        memset(year, 0, sizeof(q10_instant_statistics_t));

        g_hash_table_insert(stats, monthless, year);
    }
    instants[2] = year;

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
    GHashTable *stats = (GHashTable *) user_data;

    q10_instant_statistics_t *instants[3];
    date_t date = date_and_time_get_date(flight_get_schedule_departure_date(flight));
    if (__q10_fill_instants(stats, instants, date))
        return 1;

    for (int i = 0; i < 3; ++i)
        if (instants[i])
            instants[i]->flights++;

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
    GHashTable *stats =
        g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) free);

    for (size_t i = 0; i < n; ++i) {
        const q10_parsed_arguments_t *args = query_instance_get_argument_data(instances[i]);

        if (args->year != -1 && args->month == -1) {
            date_t date;

            for (int month = 1; month <= 12; ++month) {
                date_from_values(&date, args->year, month, 1);
                uint32_t key = date_generate_dayless(date);

                q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                if (!istats) {
                    g_hash_table_unref(stats);
                    return NULL;
                }

                memset(istats, 0, sizeof(q10_instant_statistics_t));
                g_hash_table_insert(stats, GUINT_TO_POINTER(key), istats);
            }
        } else if (args->month != -1) {
            date_t key;

            for (int day = 1; day <= 31; ++day) {
                date_from_values(&key, args->year, args->month, day);

                q10_instant_statistics_t *istats = malloc(sizeof(q10_instant_statistics_t));
                if (!istats) {
                    g_hash_table_unref(stats);
                    return NULL;
                }

                memset(istats, 0, sizeof(q10_instant_statistics_t));
                g_hash_table_insert(stats, GUINT_TO_POINTER(key), istats);
            }
        }

        /* Don't add keys for years, as its not known who those are */
    }

    flight_manager_iter(database_get_flights(database),
                        __q10_generate_statistics_foreach_flight,
                        stats);

    return stats;
}

/** @brief Start of the range of years to look for in ::__q10_execute. */
#define Q10_EXECUTE_YEAR_RANGE_START 1970
/** @brief End of the range of years to look for in ::__q10_execute. */
#define Q10_EXECUTE_YEAR_RANGE_END   2100

/**
 * @brief   Checks if a day, month or year has events happening in it.
 * @details If it hasn't, then it shouldn't be printed.
 *
 * @param istats Event count in a day / month / year.
 *
 * @return Whether there is any event in @p istats.
 */
int __q10_instant_statistics_has_events(const q10_instant_statistics_t *istats) {
    if (!istats) {
        fprintf(stderr, "Bad statistical data in query 10! This should not happen!\n");
        return 0;
    }

    return istats->users || istats->flights || istats->passengers || istats->unique_passengers ||
           istats->reservations;
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

/**
 * @brief Executes a query of type 10.
 *
 * @param database   Ignored database (all data is gathered in ::__q10_generate_statistics).
 * @param statistics Value returned ::__q10_generate_statistics.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 Always succcessful.
 * @retval 1 Bad statistical data (should not happen, please raise an issue if it does).
 */
int __q10_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;
    (void) output;

    /* TODO - fix const */
    const q10_parsed_arguments_t *args  = query_instance_get_argument_data(instance);
    GHashTable                   *stats = (GHashTable *) statistics;

    if (args->year == -1) {
        date_t date;
        for (int year = Q10_EXECUTE_YEAR_RANGE_START; year <= Q10_EXECUTE_YEAR_RANGE_END; ++year) {
            date_from_values(&date, year, 1, 1);
            uint32_t key = date_generate_monthless(date);

            const q10_instant_statistics_t *istats =
                g_hash_table_lookup(stats, GUINT_TO_POINTER(key));
            if (!istats)
                continue;
            else if (__q10_instant_statistics_has_events(istats))
                __q10_instant_statistics_write(istats, output, "year", year);
        }
    } else if (args->month == -1) {
        date_t date;

        for (int month = 1; month <= 12; ++month) {
            date_from_values(&date, args->year, month, 1);
            uint32_t key = date_generate_dayless(date);

            const q10_instant_statistics_t *istats =
                g_hash_table_lookup(stats, GUINT_TO_POINTER(key));

            if (__q10_instant_statistics_has_events(istats))
                __q10_instant_statistics_write(istats, output, "month", month);
        }
    } else if (args->month != -1) {
        date_t key;

        for (int day = 1; day <= 31; ++day) {
            date_from_values(&key, args->year, args->month, day);

            const q10_instant_statistics_t *istats =
                g_hash_table_lookup(stats, GUINT_TO_POINTER(key));

            if (__q10_instant_statistics_has_events(istats))
                __q10_instant_statistics_write(istats, output, "day", day);
        }
    }

    return 0;
}

query_type_t *q10_create(void) {
    return query_type_create(__q10_parse_arguments,
                             __q10_clone_arguments,
                             free,
                             __q10_generate_statistics,
                             (query_type_free_statistics_callback_t) g_hash_table_unref,
                             __q10_execute);
}
