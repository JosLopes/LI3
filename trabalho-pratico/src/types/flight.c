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
    char* airline;
    char* plane_model;
    char* origin;
    char* destination;
    int   schedule_departure_data;
    int   schedule_arrival_data;
    int   number_of_passengers;
    int   real_departure_time;
};

flight* create_flight (void) {
    flight* new_flight = malloc (sizeof (flight));
    new_flight -> airline = NULL;
    new_flight -> plane_model = NULL;
    new_flight -> origin = NULL;
    new_flight -> destination = NULL;
    new_flight -> number_of_passengers = 0;

    return new_flight;
}

void set_flight_airline (flight* flight, char* parsed_airline)
    {flight -> airline = strdup (parsed_airline);}

void set_flight_plane_model (flight* flight, char* parsed_plane_model)
    {flight -> plane_model = strdup (parsed_plane_model);}

void set_flight_origin (flight* flight, char* parsed_origin)
    {flight -> origin = strdup (parsed_origin);}

void set_flight_destination (flight* flight, char* parsed_destination)
    {flight -> destination = strdup (parsed_destination);}

void set_flight_schedule_departure_data (flight* flight, int parsed_schedule_data)
    {flight -> schedule_departure_data = parsed_schedule_data;}

void set_flight_schedule_arrival_data (flight* flight, int parsed_schedule_arrival_data)
    {flight -> schedule_arrival_data = parsed_schedule_arrival_data;}

void increment_flight_number_of_passengers (flight* flight, int increment_factor)
    {flight -> number_of_passengers += increment_factor;}

void set_flight_real_departure_time (flight* flight, int parsed_real_departure_time)
    {flight -> real_departure_time = parsed_real_departure_time;}

const char* get_const_flight_airline (flight* flight) {return flight -> airline;}

const char* get_const_flight_plane_model (flight* flight) {return flight -> plane_model;}

const char* get_const_flight_origin (flight* flight) {return flight -> origin;}

const char* get_const_flight_destination (flight* flight) {return flight -> destination;}

int get_flight_schedule_departure_data (flight* flight) {return flight -> schedule_departure_data;}

int get_flight_schedule_arrival_data (flight* flight) {return flight -> schedule_arrival_data;}

int get_flight_number_of_passengers (flight* flight) {return flight -> number_of_passengers;}

int get_flight_real_departure_time (flight* flight) {return flight -> real_departure_time;}

void free_flight (flight* flight) {
    free (flight -> airline);
    free (flight -> plane_model);
    free (flight -> origin);
    free (flight -> destination);

    free (flight);
}