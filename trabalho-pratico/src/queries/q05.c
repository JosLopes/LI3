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
 * @file  q05.c
 * @brief Implementation of methods in include/queries/q05.h
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "queries/q05.h"
#include "queries/query_instance.h"
#include "types/airport_code.h"
#include "utils/date_and_time.h"

typedef struct {
    GArray     *arguments_data;
    GHashTable *origin_flights;
} q05_foreach_airport_data_t;

typedef struct {
    airport_code_t  airport_code;
    date_and_time_t begin_date;
    date_and_time_t end_date;
} q05_parsed_arguments_t;

void *__q05_parse_arguments(char **argv, size_t argc) {
    if (argc != 3)
        return NULL;

    q05_parsed_arguments_t *parsed_arguments = malloc(sizeof(q05_parsed_arguments_t));
    if (!parsed_arguments)
        return NULL;

    airport_code_t airport_code;
    int            airport_retval = airport_code_from_string(&airport_code, argv[0]);
    if (airport_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->airport_code = airport_code;

    date_and_time_t begin_date;
    int             begin_date_retval = date_and_time_from_string(&begin_date, argv[1]);
    if (begin_date_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->begin_date = begin_date;

    date_and_time_t end_date;
    int             end_date_retval = date_and_time_from_string(&end_date, argv[2]);
    if (end_date_retval) {
        free(parsed_arguments);
        return NULL;
    }
    parsed_arguments->end_date = end_date;

    return parsed_arguments;
}

void __q05_free_query_instance_argument_data(void *argument_data) {
    free((q05_parsed_arguments_t *) argument_data);
}

gint __q05_generate_statistics_uint32_compare_func(gconstpointer a, gconstpointer b) {
    q05_parsed_arguments_t **parsed_arguments = (q05_parsed_arguments_t **) a;
    airport_code_t *airport_code = (airport_code_t *) b;

    return ((int32_t) (*parsed_arguments)->airport_code - (int32_t) *airport_code);
}

gint __q05_generate_statistics_uint64_compare_func(gconstpointer a, gconstpointer b) {
    return ((int64_t) * (uint64_t *) a) - ((int64_t) * (uint64_t *) b);
}

gint __q05_generate_statistics_identifiers_compare_func(gconstpointer a, gconstpointer b) {
    const flight_t *flight = *(flight_t **) a;
    uint64_t flight_id = *(uint64_t *) b; 
    
    return ((int64_t) flight_get_id(flight) - (int64_t) flight_id);
}

int __q05_generate_statistics_foreach_airport(void *user_data, const flight_t *flight) {
    q05_foreach_airport_data_t *foreach_data = (q05_foreach_airport_data_t *) user_data;

    airport_code_t  airport                 = flight_get_origin(flight);
    date_and_time_t schedule_departure_date = flight_get_schedule_departure_date(flight);

    guint airport_index;
    if (!g_array_binary_search(foreach_data->arguments_data,
                               &airport,
                               __q05_generate_statistics_uint32_compare_func,
                               &airport_index))
        return 0; /* This airport wasn't mentioned in queries */

    /* Checks if the flight scheduled departure date is in between the given dates */
    q05_parsed_arguments_t *argument_data =
        g_array_index(foreach_data->arguments_data, q05_parsed_arguments_t *, airport_index);
    
    date_and_time_t end_date   = argument_data->end_date;
    date_and_time_t begin_date = argument_data->begin_date;
    if (date_and_time_diff(begin_date, schedule_departure_date) > 0 ||
        date_and_time_diff(end_date, schedule_departure_date) < 0)
        return 0;

    /* TODO - using only airport isn't unique enough */
    GArray *flights =
        g_hash_table_lookup(foreach_data->origin_flights, GUINT_TO_POINTER(airport));

    if (!flights) {
        flights = g_array_new(FALSE, FALSE, sizeof(flight_t *));
        if (!flights)
            return 1; /* Allocation failure */

        g_hash_table_insert(foreach_data->origin_flights, GUINT_TO_POINTER(airport), flights);
    }

    /* TODO - Adds a flight if its not already in the array */
    g_array_append_val(flights, flight);

    return 0;
}

gint __q05_sort_flights_by_date(gconstpointer a, gconstpointer b) {
    const flight_t *flight_a = *((flight_t **) a);
    const flight_t *flight_b = *((flight_t **) b);

    int64_t diff = date_and_time_diff(flight_get_schedule_departure_date(flight_b),
                                      flight_get_schedule_departure_date(flight_a));

    if (diff == 0)
        return flight_get_id(flight_a) - flight_get_id(flight_b);
    else
        return diff;
}

void __q05_generate_statistics_sort_each_array(gpointer key, gpointer value, gpointer user_data) {
    (void) key;
    (void) user_data;

    g_array_sort((GArray *) value, __q05_sort_flights_by_date);
}

gint __q05_generate_statistics_parsed_arguments_compare_func(gconstpointer a, gconstpointer b) {
    q05_parsed_arguments_t *argument_data_a = *(q05_parsed_arguments_t **) a;
    q05_parsed_arguments_t *argument_data_b = *(q05_parsed_arguments_t **) b;

    return ((int64_t) argument_data_a->airport_code - (int64_t) argument_data_b->airport_code);
}

void *__q05_generate_statistics(database_t *database, query_instance_t *instances, size_t n) {
    (void) database;

    /*
     * Generate an array of all airport codes that will be asked for in queries, to avoid storing
     * information about all airports, and to save information about the required date time frame.
     */
    GArray *arguments_data = g_array_new(FALSE, FALSE, sizeof(q05_parsed_arguments_t *));
    for (size_t i = 0; i < n; ++i) {
        q05_parsed_arguments_t *argument_data =
            (q05_parsed_arguments_t *) query_instance_get_argument_data(instances);
        g_array_append_val(arguments_data, argument_data);
        instances = (query_instance_t *) ((uint8_t *) instances + query_instance_sizeof());
    }
    g_array_sort(arguments_data, __q05_generate_statistics_parsed_arguments_compare_func);

    GHashTable *origin_flights = g_hash_table_new(g_direct_hash, g_direct_equal);

    q05_foreach_airport_data_t callback_data = {.arguments_data = arguments_data,
                                                .origin_flights = origin_flights};

    if (flight_manager_iter(database_get_flights(database),
                            __q05_generate_statistics_foreach_airport,
                            &callback_data)) { /* Allocation failure */

        g_array_free(arguments_data, TRUE);
        g_hash_table_destroy(origin_flights);
        return NULL;
    }
    g_array_free(arguments_data, TRUE);

    g_hash_table_foreach(origin_flights, __q05_generate_statistics_sort_each_array, NULL);
    return origin_flights;
}

void __q05_free_hashtable_values(gpointer key, gpointer value, gpointer user_data) {
    (void) key;
    (void) user_data;

    g_array_unref((GArray *) value);
}

void __q05_free_statistics(void *statistics) {
    GHashTable *origin_flights = (GHashTable*) statistics; 

    g_hash_table_foreach(origin_flights, __q05_free_hashtable_values, NULL);
    g_hash_table_destroy(origin_flights);
}

int __q05_execute(database_t       *database,
                  void             *statistics,
                  query_instance_t *instance,
                  FILE             *output) {
    (void) database;

    GHashTable             *origin_flights = (GHashTable *) statistics;
    q05_parsed_arguments_t *arguments =
        (q05_parsed_arguments_t *) query_instance_get_argument_data(instance);

    GArray *flights =
        g_hash_table_lookup(origin_flights, GUINT_TO_POINTER(arguments->airport_code));
    if (!flights) {
        fprintf(stderr, "Bad statistical data in query 5! This should not happen!\n");
        return 1;
    }
    size_t flights_len = flights->len;

    if (query_instance_get_formatted(instance)) {
        for (size_t i = 0; i < flights_len; i++) {
            flight_t *flight = g_array_index(flights, flight_t*, i);

            char scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
            date_and_time_sprintf(scheduled_departure_str,
                                  flight_get_schedule_departure_date(flight));

            char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
            airport_code_sprintf(destination_airport, flight_get_destination(flight));

            fprintf(output,
                    "--- %ld ---\nid: %010" PRIu64 "\nschedule_departure_date: %s\n"
                    "destination: %s\nairline: %s\nplane_model: %s\n",
                    i+1,
                    flight_get_id(flight),
                    scheduled_departure_str,
                    destination_airport,
                    flight_get_const_airline(flight),
                    flight_get_const_plane_model(flight));
        
            if (i != flights_len - 1)
                fputc('\n', output);
        }
    } else {
        for (size_t i = 0; i < flights_len; i++) {
            flight_t *flight = g_array_index(flights, flight_t*, i);

            char scheduled_departure_str[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
            date_and_time_sprintf(scheduled_departure_str,
                                  flight_get_schedule_departure_date(flight));

            char destination_airport[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
            airport_code_sprintf(destination_airport, flight_get_destination(flight));

            fprintf(output,
                    "%010" PRIu64 ";%s;%s;%s;%s\n",
                    flight_get_id(flight),
                    scheduled_departure_str,
                    destination_airport,
                    flight_get_const_airline(flight),
                    flight_get_const_plane_model(flight));
        }
    }

    return 0;
}

query_type_t *q05_create(void) {
    return query_type_create(__q05_parse_arguments,
                             __q05_free_query_instance_argument_data,
                             __q05_generate_statistics,
                             __q05_free_statistics,
                             __q05_execute);
}
