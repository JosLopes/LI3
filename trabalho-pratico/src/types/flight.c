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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "types/flight.h"

struct flight {
    char           *airline;
    char           *plane_model;
    char           *origin;
    char           *destination;
    size_t          id;
    date_and_time_t schedule_departure_date;
    date_and_time_t real_departure_date;
    date_and_time_t schedule_arrival_date;
    int             number_of_passengers;
};

flight_t *flight_create(void) {
    flight_t *new_flight             = malloc(sizeof(flight_t));
    new_flight->airline              = NULL;
    new_flight->plane_model          = NULL;
    new_flight->origin               = NULL;
    new_flight->destination          = NULL;
    new_flight->number_of_passengers = 0;

    return new_flight;
}

void flight_set_airline(flight_t *flight, char *airline) {
    flight->airline = strdup(airline);
}

void flight_set_plane_model(flight_t *flight, char *plane_model) {
    flight->plane_model = strdup(plane_model);
}

void flight_set_origin(flight_t *flight, char *origin) {
    flight->origin = strdup(origin);
}

void flight_set_destination(flight_t *flight, char *destination) {
    flight->destination = strdup(destination);
}

void flight_set_id(flight_t *flight, size_t id) {
    flight->id = id;
}

void flight_set_schedule_departure_date(flight_t       *flight,
                                        date_and_time_t scheduled_departure_date) {
    flight->schedule_departure_date = scheduled_departure_date;
}

void flight_set_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date) {
    flight->schedule_arrival_date = schedule_arrival_date;
}

void increment_flight_number_of_passengers(flight_t *flight, int increment_factor) {
    flight->number_of_passengers += increment_factor;
}

void flight_set_number_of_passengers(flight_t *flight, int number_of_passengers) {
    flight->number_of_passengers = number_of_passengers;
}

void flight_set_real_departure_date(flight_t *flight, date_and_time_t real_departure_date) {
    flight->real_departure_date = real_departure_date;
}

const char *flight_get_const_airline(flight_t *flight) {
    return flight->airline;
}

const char *flight_get_const_plane_model(flight_t *flight) {
    return flight->plane_model;
}

const char *flight_get_const_origin(flight_t *flight) {
    return flight->origin;
}

const char *flight_get_const_destination(flight_t *flight) {
    return flight->destination;
}

size_t flight_get_id(flight_t *flight) {
    return flight->id;
}

date_and_time_t flight_get_schedule_departure_date(flight_t *flight) {
    return flight->schedule_departure_date;
}

date_and_time_t flight_get_schedule_arrival_date(flight_t *flight) {
    return flight->schedule_arrival_date;
}

int flight_get_number_of_passengers(flight_t *flight) {
    return flight->number_of_passengers;
}

date_and_time_t flight_get_real_departure_date(flight_t *flight) {
    return flight->real_departure_date;
}

void flight_free(flight_t *flight) {
    free(flight->airline);
    free(flight->plane_model);
    free(flight->origin);
    free(flight->destination);

    free(flight);
}
