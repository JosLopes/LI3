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
 * @file     flight.h
 * @brief    Declaration of type `flight_t`.
 *
 * @details  In this module you can find a declaration of the type `flight_t` as a struct flight,
 *           as well as get and set functions, which allow's the developer access to any
 *           previously created flight, or gives them the ability to create a new flight.
 *           Flight parameters available for the developer:
 *
 * @param airline                 Airline of a given flight, of type `char*`.
 * @param plane_model             Plane_model of a given flight, of type `char*`.
 * @param origin                  Origin of a given flight, of type `char*`.
 * @param destination             Destination of a given flight, of type `char*`.
 * @param id                      Id of a given flight, of type `size_t`. 
 * @param schedule_departure_date Scheduled departure date of a given flight,
 *                                of type `date_and_time_t`.
 * @param real_departure_date     Real departure date of a given flight, of type `date_and_time_t`.
 * @param schedule_arrival_date   Scheduled arrival date of a given flight,
 *                                of type `date_and_time_t`.
 * @param number_of_passengers    Number of passengers in a flight, of type `int`. 
 */

#ifndef FLIGHT_H
#define FLIGHT_H

#include <stddef.h>

#include "utils/date_and_time.h"

/**
 * @brief Type `flight_t` defined as a struct flight,
 *        stores valuable information of a given flight (Opaque type).
 */
typedef struct flight flight_t;

/**
 * @brief Creates a new flight.
 *
 * @return new_flight, the new flight created of type `flight_t*`.
 */
flight_t *create_flight(void);

/**
 * @brief Sets the flight's airline, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_flight).
 *
 * @param flight  Flight of type `flight_t*`.
 * @param airline Airline of a given flight, of type `char*`.
 */
void set_flight_airline(flight_t *flight, char *airline);

/**
 * @brief Sets the flight's plane model, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_flight).
 *
 * @param flight      Flight of type `flight_t*`.
 * @param plane_model Plane model of a given flight, of type `char*`.
 */
void set_flight_plane_model(flight_t *flight, char *plane_model);

/**
 * @brief Sets the flight's origin, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_flight).
 *
 * @param flight Flight of type `flight_t*`.
 * @param origin Origin of a given flight, of type `char*`.
 */
void set_flight_origin(flight_t *flight, char *origin);

/**
 * @brief Sets the flight's destination, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_flight).
 *
 * @param flight      Flight of type `flight_t*`.
 * @param destination Plane destination of a given flight, of type `char*`.
 */
void set_flight_destination(flight_t *flight, char *destination);

/**
 * @brief Sets the flight's id.
 *
 * @param flight Flight of type `flight_t*`.
 * @param id     Id of a given flight, of type `size_t`.
 */
void set_flight_id(flight_t *flight, size_t id);

/**
 * @brief Sets the flight's departure date.
 *
 * @param flight                  Flight of type `flight_t*`.
 * @param schedule_departure_date Scheduled departure date of a given flight,
 *                                of type `date_and_time_t`.
 */
void set_flight_schedule_departure_date(flight_t *flight, date_and_time_t schedule_departure_date);

/**
 * @brief Sets the flight's schedule_arrival_date.
 *
 * @param flight                Flight of type `flight_t*`.
 * @param schedule_arrival_date Scheduled arrival date of a given flight, of type `date_and_time_t`.
 */
void set_flight_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date);

/**
 * @brief Increments the flight's number_of_passengers.
 *
 * @param flight            Flight of type `flight_t*`.
 * @param increment_factor  The factor by which the number of passengers in a flight will
 *                          be incremented, of type `int`.
 */
void increment_flight_number_of_passengers(flight_t *flight, int increment_factor);

/**
 * @brief Sets the flight's number of passengers.
 *
 * @param flight               Flight of type `flight_t`.
 * @param number_of_passengers Number of passengers of a given flight, of type `int`.
 */
void set_flight_number_of_passengers(flight_t *flight, int number_of_passengers);

/**
 * @brief Sets the flight's real_departure_time.
 *
 * @param flight              Flight of type `flight_t*`.
 * @param real_departure_date Real departure date of a given flight, of type `date_and_time_t`.
 */
void set_flight_real_departure_date(flight_t *flight, date_and_time_t real_departure_date);

/**
 * @brief Gets the flight's airline.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's airline, of type `const char*`.
 */
const char *get_const_flight_airline(flight_t *flight);

/**
 * @brief Gets the flight's plane model.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's plane_model, of type `const char*`.
 */
const char *get_const_flight_plane_model(flight_t *flight);

/**
 * @brief Gets the flight's origin.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's origin, of type `const char*`.
 */
const char *get_const_flight_origin(flight_t *flight);

/**
 * @brief Gets the flight's destination.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's destination, of type `const char*`.
 */
const char *get_const_flight_destination(flight_t *flight);

/**
 * @brief Gets the flight's id.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's id, of type `size_t`.
 */
size_t get_flight_id(flight_t *flight);

/**
 * @brief Gets the flight's departure date.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's schedule_departure_date, of type `date_and_time_t`.
 */
date_and_time_t get_flight_schedule_departure_date(flight_t *flight);

/**
 * @brief Gets the flight's arrival date.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's schedule_arrival_date, of type `date_and_time_t`.
 */
date_and_time_t get_flight_schedule_arrival_date(flight_t *flight);

/**
 * @brief Gets the flight's passengers.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's airline, of type `int`.
 */
int get_flight_number_of_passengers(flight_t *flight);

/**
 * @brief Gets the flight's real departure date.
 * @param flight Flight of type `flight_t*`.
 * @return The flight's real_departure_date, of type `date_and_time_t`.
 */
date_and_time_t get_flight_real_departure_date(flight_t *flight);

/**
 * @brief Function that frees the memory used for a given flight, effectively deleting the flight.
 * @param flight Flight of type `flight_t*`.
 */
void free_flight(flight_t *flight);

#endif
