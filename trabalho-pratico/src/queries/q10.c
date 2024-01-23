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

#include <glib.h>
#include <inttypes.h>
#include <stdlib.h>

#include "queries/q10.h"
#include "queries/query_instance.h"
#include "utils/int_utils.h"

typedef struct {
    int16_t year;
    int8_t  month;
} q10_parsed_arguments_t;

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

void *__q10_clone_arguments(const void *args_data) {
    const q10_parsed_arguments_t *args  = args_data;
    q10_parsed_arguments_t       *clone = malloc(sizeof(q10_parsed_arguments_t));
    if (!clone)
        return NULL;

    memcpy(clone, args, sizeof(q10_parsed_arguments_t));
    return clone;
}

typedef struct {
    uint64_t users, flights, passengers, unique_passengers, reservations;
    int16_t  year;
    int8_t   month;
} q10_instant_statistics_t;

typedef struct {
    q10_instant_statistics_t **stats_data;
    const flight_manager_t    *flights;
    GHashTable                *aux_counted;
    size_t                     stats_length;
} q10_foreach_user_data_t;

int __q10_instant_exists(int16_t year, int8_t month, q10_instant_statistics_t *istats) {
    if (year == istats->year && istats->month == -1)
        return 12;

    if (year == istats->year && month == istats->month)
        return 31;

    if (istats->year == -1 && istats->month == -1)
        return 1;

    return 0;
}

/** @brief Start of the range of years to look for in ::__q10_execute. */
#define Q10_EXECUTE_YEAR_RANGE_START 1970
/** @brief End of the range of years to look for in ::__q10_execute. */
#define Q10_EXECUTE_YEAR_RANGE_END   2100

void __q10_add_passenger(GHashTable *table, date_t key, q10_instant_statistics_t *istats) {
    if (!g_hash_table_lookup(table, GUINT_TO_POINTER(key))) {
        istats->unique_passengers++;

        g_hash_table_insert(table, GUINT_TO_POINTER(key), GUINT_TO_POINTER(1));
    }

    istats->passengers++;
}

int __q10_generate_statistics_foreach_user(void                               *user_data,
                                           const user_t                       *user,
                                           const single_pool_id_linked_list_t *passengers) {
    q10_foreach_user_data_t *iter_data = user_data;

    date_t   date  = user_get_account_creation_date(user);
    uint16_t year  = date_get_year(date);
    uint8_t  month = date_get_month(date);
    uint8_t  day   = date_get_day(date);

    for (size_t i = 0; i < iter_data->stats_length; ++i) {
        int retval = __q10_instant_exists(year, month, (iter_data->stats_data)[i]);
        if (retval == 31)
            ((iter_data->stats_data)[i][day]).users++;
        else if (retval == 12)
            ((iter_data->stats_data)[i][month]).users++;
        else if (retval == 1)
            ((iter_data->stats_data)[i][year - Q10_EXECUTE_YEAR_RANGE_START + 1]).users++;
    }

    g_hash_table_remove_all(iter_data->aux_counted);
    while (passengers) {
        const flight_t *flight =
            flight_manager_get_by_id(iter_data->flights,
                                     single_pool_id_linked_list_get_value(passengers));
        date  = date_and_time_get_date(flight_get_schedule_departure_date(flight));
        year  = date_get_year(date);
        month = date_get_month(date);
        day   = date_get_day(date);

        for (size_t i = 0; i < iter_data->stats_length; ++i) {
            int retval = __q10_instant_exists(year, month, (iter_data->stats_data)[i]);

            if (retval == 1) {
                __q10_add_passenger(
                    iter_data->aux_counted,
                    date_generate_monthless(date),
                    &((iter_data->stats_data)[i][year - Q10_EXECUTE_YEAR_RANGE_START + 1]));
            } else if (retval == 31) {
                __q10_add_passenger(iter_data->aux_counted,
                                    date,
                                    &((iter_data->stats_data)[i][day]));
            } else if (retval == 12) {
                __q10_add_passenger(iter_data->aux_counted,
                                    date_generate_dayless(date),
                                    &((iter_data->stats_data)[i][month]));
            }
        }

        passengers = single_pool_id_linked_list_get_next(passengers);
    }

    return 0;
}

typedef struct {
    q10_instant_statistics_t **stats_data;
    size_t                     stats_length;
} q10_statistics_helper_t;

int __q10_generate_statistics_foreach_flight(void *user_data, const flight_t *flight) {
    q10_statistics_helper_t *iter_data = (q10_statistics_helper_t *) user_data;

    date_t   date  = date_and_time_get_date(flight_get_schedule_departure_date(flight));
    uint16_t year  = date_get_year(date);
    uint8_t  month = date_get_month(date);
    uint8_t  day   = date_get_day(date);

    for (size_t i = 0; i < iter_data->stats_length; ++i) {
        int retval = __q10_instant_exists(year, month, (iter_data->stats_data)[i]);
        if (retval == 31)
            ((iter_data->stats_data)[i][day]).flights++;
        else if (retval == 12)
            ((iter_data->stats_data)[i][month]).flights++;
        else if (retval == 1)
            ((iter_data->stats_data)[i][year - Q10_EXECUTE_YEAR_RANGE_START + 1]).flights++;
    }

    return 0;
}

int __q10_generate_statistics_foreach_reservation(void                *user_data,
                                                  const reservation_t *reservation) {
    q10_statistics_helper_t *iter_data = (q10_statistics_helper_t *) user_data;

    date_t   date  = reservation_get_begin_date(reservation);
    uint16_t year  = date_get_year(date);
    uint8_t  month = date_get_month(date);
    uint8_t  day   = date_get_day(date);

    for (size_t i = 0; i < iter_data->stats_length; ++i) {
        int retval = __q10_instant_exists(year, month, (iter_data->stats_data)[i]);
        if (retval == 31)
            ((iter_data->stats_data)[i][day]).reservations++;
        else if (retval == 12)
            ((iter_data->stats_data)[i][month]).reservations++;
        else if (retval == 1)
            ((iter_data->stats_data)[i][year - Q10_EXECUTE_YEAR_RANGE_START + 1]).reservations++;
    }

    return 0;
}

q10_instant_statistics_t *
    __q10_allocate_instant_stats_array(size_t size, int16_t year, int8_t month) {
    q10_instant_statistics_t *new = malloc(size * sizeof(q10_instant_statistics_t));
    if (!new)
        return NULL;

    memset(new, 0, size * sizeof(q10_instant_statistics_t));
    new->year  = year;
    new->month = month;

    return new;
}

void *__q10_generate_statistics(const database_t              *database,
                                const query_instance_t *const *instances,
                                size_t                         n) {

    q10_instant_statistics_t **stats_data = malloc(n * sizeof(q10_instant_statistics_t *));

    size_t array_size = n;
    for (size_t i = 0; i < n; ++i) {
        const q10_parsed_arguments_t *args = query_instance_get_argument_data(instances[i]);

        size_t j;
        for (j = 0; j < i; ++j)
            if (args->month == (stats_data[j])->month && args->year == (stats_data[j])->year)
                break;
        if (j != i) {
            --array_size;
            continue;
        }

        if (args->year != -1 && args->month == -1) {
            stats_data[i] = __q10_allocate_instant_stats_array(13, args->year, -1);
            if (!stats_data[i])
                return NULL;
        } else if (args->month != -1) {
            stats_data[i] = __q10_allocate_instant_stats_array(32, args->year, args->month);
            if (!stats_data[i])
                return NULL;
        } else if (args->year == -1 && args->month == -1) {
            stats_data[i] = __q10_allocate_instant_stats_array(Q10_EXECUTE_YEAR_RANGE_END -
                                                                   Q10_EXECUTE_YEAR_RANGE_START + 1,
                                                               -1,
                                                               -1);
            if (!stats_data[i])
                return NULL;
        }
    }

    q10_foreach_user_data_t user_iter_data = {.stats_data = stats_data,
                                              .flights    = database_get_flights(database),
                                              .aux_counted =
                                                  g_hash_table_new(g_direct_hash, g_direct_equal),
                                              .stats_length = array_size};

    user_manager_iter_with_flights(database_get_users(database),
                                   __q10_generate_statistics_foreach_user,
                                   &user_iter_data);
    g_hash_table_unref(user_iter_data.aux_counted);

    q10_statistics_helper_t *stats = malloc(sizeof(q10_statistics_helper_t));
    if (!stats)
        return NULL;

    stats->stats_data   = stats_data;
    stats->stats_length = array_size;

    flight_manager_iter(database_get_flights(database),
                        __q10_generate_statistics_foreach_flight,
                        stats);

    reservation_manager_iter(database_get_reservations(database),
                             __q10_generate_statistics_foreach_reservation,
                             stats);

    return stats;
}

void __q10_instant_statistics_write(const q10_instant_statistics_t *istats_data,
                                    query_writer_t                 *output,
                                    const char                     *ymd,
                                    int                             value) {

    query_writer_write_new_object(output);
    query_writer_write_new_field(output, ymd, "%d", value);
    query_writer_write_new_field(output, "users", "%" PRIu64, istats_data->users);
    query_writer_write_new_field(output, "flights", "%" PRIu64, istats_data->flights);
    query_writer_write_new_field(output, "passengers", "%" PRIu64, istats_data->passengers);
    query_writer_write_new_field(output,
                                 "unique_passengers",
                                 "%" PRIu64,
                                 istats_data->unique_passengers);
    query_writer_write_new_field(output, "reservations", "%" PRIu64, istats_data->reservations);
}

int __q10_for_display(int16_t year, int8_t month, q10_instant_statistics_t *istats) {
    if (year == istats->year && year != -1 && istats->month == -1 && month == -1)
        return 12;

    if (year == istats->year && year != -1 && month == istats->month && month != -1)
        return 31;

    if (year == -1 && month == -1 && istats->year == -1 && istats->month == -1)
        return 1;

    return 0;
}

int __q10_instant_statistics_has_events(const q10_instant_statistics_t *istats) {
    if (!istats) {
        fprintf(stderr, "Bad statistical data in query 10! This should not happen!\n");
        return 0;
    }

    return istats->users || istats->flights || istats->passengers || istats->unique_passengers ||
           istats->reservations;
}

int __q10_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const q10_parsed_arguments_t *args  = query_instance_get_argument_data(instance);
    q10_statistics_helper_t      *stats = (q10_statistics_helper_t *) statistics;

    for (size_t i = 0; i < stats->stats_length; ++i) {
        int retval = __q10_for_display(args->year, args->month, (stats->stats_data)[i]);
        if (retval == 31) {
            for (int j = 1; j <= retval; ++j) {
                const q10_instant_statistics_t *istats = stats->stats_data[i];
                if (__q10_instant_statistics_has_events(&(istats[j])))
                    __q10_instant_statistics_write(&istats[j], output, "day", j);
            }
        } else if (retval == 12) {
            for (int j = 1; j <= retval; ++j) {
                const q10_instant_statistics_t *istats = stats->stats_data[i];
                if (__q10_instant_statistics_has_events(&(istats[j])))
                    __q10_instant_statistics_write(&istats[j], output, "month", j);
            }
        } else if (retval == 1) {
            for (int j = 1; j <= Q10_EXECUTE_YEAR_RANGE_END - Q10_EXECUTE_YEAR_RANGE_START; ++j) {
                const q10_instant_statistics_t *istats = stats->stats_data[i];
                if (__q10_instant_statistics_has_events(&(istats[j])))
                    __q10_instant_statistics_write(&istats[j],
                                                   output,
                                                   "year",
                                                   j + Q10_EXECUTE_YEAR_RANGE_START - 1);
            }
        }
    }

    return 0;
}

void __q10_free_statistics(void *statistics) {
    q10_statistics_helper_t *stats = (q10_statistics_helper_t *) statistics;

    for (size_t i = 0; i < stats->stats_length; ++i)
        free((stats->stats_data)[i]);

    free(stats->stats_data);
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
