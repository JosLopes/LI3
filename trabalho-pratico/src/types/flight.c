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

#include "types/flight.h"
#include <stdlib.h>
#include <string.h>

struct flight {
    char *airline;
    char *plane_model;
    char *origin;
    char *destination;
    int   id;
    int   schedule_departure_date;
    int   real_departure_time;
    int   schedule_arrival_date;
    int   number_of_passengers;
};

flight_t *create_flight(void) {
    flight_t *new_flight             = malloc(sizeof(flight_t));
    new_flight->airline              = NULL;
    new_flight->plane_model          = NULL;
    new_flight->origin               = NULL;
    new_flight->destination          = NULL;
    new_flight->number_of_passengers = 0;

    return new_flight;
}

void set_flight_airline(flight_t *flight, char *airline) {
    flight->airline = strdup(airline);
}

void set_flight_plane_model(flight_t *flight, char *plane_model) {
    flight->plane_model = strdup(plane_model);
}

void set_flight_origin(flight_t *flight, char *origin) {
    flight->origin = strdup(origin);
}

void set_flight_destination(flight_t *flight, char *destination) {
    flight->destination = strdup(destination);
}

void set_flight_id(flight_t *flight, int id) {
    flight->id = id;
}

void set_flight_schedule_departure_date(flight_t *flight, int scheduled_departure_date) {
    flight->schedule_departure_date = scheduled_departure_date;
}

void set_flight_schedule_arrival_date(flight_t *flight, int schedule_arrival_date) {
    flight->schedule_arrival_date = schedule_arrival_date;
}

void increment_flight_number_of_passengers(flight_t *flight, int increment_factor) {
    flight->number_of_passengers += increment_factor;
}

void set_flight_number_of_passengers(flight_t *flight, int number_of_passengers) {
    flight->number_of_passengers = number_of_passengers;
}

void set_flight_real_departure_date(flight_t *flight, int real_departure_time) {
    flight->real_departure_time = real_departure_time;
}

const char *get_const_flight_airline(flight_t *flight) {
    return flight->airline;
}

const char *get_const_flight_plane_model(flight_t *flight) {
    return flight->plane_model;
}

const char *get_const_flight_origin(flight_t *flight) {
    return flight->origin;
}

const char *get_const_flight_destination(flight_t *flight) {
    return flight->destination;
}

int get_flight_id(flight_t *flight) {
    return flight->id;
}

int get_flight_schedule_departure_date(flight_t *flight) {
    return flight->schedule_departure_date;
}

int get_flight_schedule_arrival_date(flight_t *flight) {
    return flight->schedule_arrival_date;
}

int get_flight_number_of_passengers(flight_t *flight) {
    return flight->number_of_passengers;
}

int get_flight_real_departure_time(flight_t *flight) {
    return flight->real_departure_time;
}

void free_flight(flight_t *flight) {
    free(flight->airline);
    free(flight->plane_model);
    free(flight->origin);
    free(flight->destination);

    free(flight);
}
