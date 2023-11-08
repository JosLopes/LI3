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
 * @file test.c
 * @brief Contains the entry point to the program.
 */

#include <stdio.h>

#include "database/flight_manager.h"
#include "dataset/dataset_loader.h"
#include "types/flight.h"
#include "utils/pool.h"

int iter_callback(void *user_data, flight_t *flight) {
    (void) user_data;

    size_t      id          = flight_get_id(flight);
    const char *airline     = flight_get_const_airline(flight);
    const char *passport    = flight_get_const_plane_model(flight);
    int         total_seats = flight_get_total_seats(flight);

    char origin[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(origin, flight_get_origin(flight));

    char destination[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
    airport_code_sprintf(destination, flight_get_destination(flight));

    char schedule_departure_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(schedule_departure_date, flight_get_schedule_departure_date(flight));

    char schedule_arrival_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(schedule_arrival_date, flight_get_schedule_arrival_date(flight));

    char real_departure_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(real_departure_date, flight_get_real_departure_date(flight));

    printf("--- FLIGHT ---\nid: %zu\nairline: %s\nplane_model: %s\ntotal_seats: %d\norigin: %s"
           "\ndestination: %s\nschedule_departure_date: %s\nschedule_arrival_date: %s\n"
           "real_departure_date: %s\n\n",
           id,
           airline,
           passport,
           total_seats,
           origin,
           destination,
           schedule_departure_date,
           schedule_arrival_date,
           real_departure_date);

    return 0; // You can return a value other than 0 to order iteration to stop
}

int main() {
    database_t *database = database_create();
    if (!database) {
        fprintf(stderr, "Failed to allocate database!");
        return 1;
    }

    if (dataset_loader_load(database, "/home/voidbert/Uni/3/LI3/dataset/data")) {
        fputs("Failed to open dataset to be parsed.\n", stderr);
        return 1;
    }

    flight_manager_iter(database_get_flights(database), iter_callback, NULL);

    database_free(database);
    return 0;
}
