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

#ifndef FLIGHT_H
#define FLIGHT_H

typedef struct flight flight;

flight* create_flight (void);

void set_flight_airline (flight* flight, char* parsed_airline);

void set_flight_plane_model (flight* flight, char* parsed_plane_model);

void set_flight_origin (flight* flight, char* parsed_origin);

void set_flight_destination (flight* flight, char* parsed_destination);

void set_flight_schedule_departure_date (flight* flight, int parsed_schedule_date);

void set_flight_schedule_arrival_date (flight* flight, int parsed_schedule_arrival_date);

void increment_flight_number_of_passengers (flight* flight, int increment_factor);

void set_flight_real_departure_time (flight* flight, int parsed_real_departure_time);

const char* get_const_flight_airline (flight* flight);

const char* get_const_flight_plane_model (flight* flight);

const char* get_const_flight_origin (flight* flight);

const char* get_const_flight_destination (flight* flight);

int get_flight_schedule_departure_date (flight* flight);

int get_flight_schedule_arrival_date (flight* flight);

int get_flight_number_of_passengers (flight* flight);

int get_flight_real_departure_time (flight* flight);

void free_flight (flight* flight);

#endif