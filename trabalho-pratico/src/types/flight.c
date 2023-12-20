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
 * @file  flight.c
 * @brief Implementation of methods in include/types/flight.h
 *
 * #### Example
 * See [the header file's documentation](@ref flight_examples).
 */
#include <stdlib.h>
#include <string.h>

#include "types/flight.h"

/**
 * @struct flight
 * @brief Represents a flight in the datasets.
 * @details NOTE: some fields in the project's requirements (such as real arrival date, pilot,
 *          copilot and notes) aren't put here, as they won't be required by any
 *          database query.
 *
 * @var flight::airline
 *     @brief Airline of a given flight.
 * @var flight::plane_model
 *     @brief Plane model of a given flight.
 * @var flight::origin
 *     @brief Airport of origin of the flight.
 * @var flight::destination
 *     @brief Airport of origin of the flight.
 * @var flight::id
 *     @brief Identifier of a given flight.
 * @var flight::schedule_departure_date
 *     @brief Scheduled departure date of a given flight.
 * @var flight::real_departure_date
 *     @brief Real departure date of a given flight.
 * @var flight::schedule_arrival_date
 *     @brief Scheduled arrival date of a given flight.
 * @var flight::number_of_passengers
 *     @brief Number of passengers of a given flight.
 * @var flight::total_seats
 *     @brief Number of total seats of a given flight.
 * @var flight::owns_itself
 *     @brief   Whether, when `free`ing this flight, the flight pointer should be `free`'d.
 *     @details A false value means that the flight is allocated in a pool.
 * @var flight::owns_airline
 *     @brief Whether ::flight::airline should be `free`d.
 * @var flight::owns_plane_model
 *     @brief Whether ::flight::plane_model should be `free`d.
 */
struct flight {
    const char     *airline;
    const char     *plane_model;
    airport_code_t  origin;
    airport_code_t  destination;
    flight_id_t     id;
    date_and_time_t schedule_departure_date;
    date_and_time_t real_departure_date;
    date_and_time_t schedule_arrival_date;
    int             number_of_passengers;
    int             total_seats;

    int owns_itself, owns_airline, owns_plane_model;
};

flight_t *flight_create(pool_t *allocator) {
    flight_t *ret = allocator ? pool_alloc_item(flight_t, allocator) : malloc(sizeof(flight_t));
    if (!ret)
        return NULL;

    ret->owns_itself  = allocator == NULL;
    ret->owns_airline = ret->owns_plane_model = 0; /* Don't free in first setter call */
    return ret;
}

flight_t *flight_clone(pool_t                      *allocator,
                       string_pool_no_duplicates_t *string_allocator,
                       const flight_t              *flight) {

    flight_t *ret = flight_create(allocator);
    if (!ret)
        return NULL;

    memcpy(ret, flight, sizeof(flight_t));
    ret->owns_itself  = allocator == NULL;
    ret->owns_airline = ret->owns_plane_model = 0; /* Don't free in first setter call */

    flight_set_airline(string_allocator, ret, flight->airline);
    flight_set_plane_model(string_allocator, ret, flight->plane_model);

    return ret;
}

void flight_set_airline(string_pool_no_duplicates_t *allocator,
                        flight_t                    *flight,
                        const char                  *airline) {

    const char *new_airline =
        allocator ? string_pool_no_duplicates_put(allocator, airline) : strdup(airline);
    if (flight->owns_airline)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->airline);
    flight->owns_airline = allocator == NULL;

    flight->airline = new_airline;
}

void flight_set_plane_model(string_pool_no_duplicates_t *allocator,
                            flight_t                    *flight,
                            const char                  *plane_model) {

    const char *new_plane_model =
        allocator ? string_pool_no_duplicates_put(allocator, plane_model) : strdup(plane_model);
    if (flight->owns_plane_model)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->plane_model);
    flight->owns_plane_model = allocator == NULL;

    flight->plane_model = new_plane_model;
}

void flight_set_origin(flight_t *flight, airport_code_t origin) {
    flight->origin = origin;
}

void flight_set_destination(flight_t *flight, airport_code_t destination) {
    flight->destination = destination;
}

void flight_set_id(flight_t *flight, flight_id_t id) {
    flight->id = id;
}

void flight_set_schedule_departure_date(flight_t       *flight,
                                        date_and_time_t scheduled_departure_date) {
    flight->schedule_departure_date = scheduled_departure_date;
}

void flight_set_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date) {
    flight->schedule_arrival_date = schedule_arrival_date;
}

void flight_increment_number_of_passengers(flight_t *flight, int increment_factor) {
    flight->number_of_passengers += increment_factor;
}

void flight_set_number_of_passengers(flight_t *flight, int number_of_passengers) {
    flight->number_of_passengers = number_of_passengers;
}

void flight_set_real_departure_date(flight_t *flight, date_and_time_t real_departure_date) {
    flight->real_departure_date = real_departure_date;
}

void flight_set_total_seats(flight_t *flight, int total_seats) {
    flight->total_seats = total_seats;
}

const char *flight_get_const_airline(const flight_t *flight) {
    return flight->airline;
}

const char *flight_get_const_plane_model(const flight_t *flight) {
    return flight->plane_model;
}

airport_code_t flight_get_origin(const flight_t *flight) {
    return flight->origin;
}

airport_code_t flight_get_destination(const flight_t *flight) {
    return flight->destination;
}

flight_id_t flight_get_id(const flight_t *flight) {
    return flight->id;
}

date_and_time_t flight_get_schedule_departure_date(const flight_t *flight) {
    return flight->schedule_departure_date;
}

date_and_time_t flight_get_schedule_arrival_date(const flight_t *flight) {
    return flight->schedule_arrival_date;
}

int flight_get_number_of_passengers(const flight_t *flight) {
    return flight->number_of_passengers;
}

date_and_time_t flight_get_real_departure_date(const flight_t *flight) {
    return flight->real_departure_date;
}

int flight_get_total_seats(const flight_t *flight) {
    return flight->total_seats;
}

size_t flight_sizeof(void) {
    return sizeof(struct flight);
}

int flight_is_valid(const flight_t *flight) {
    return flight->id == (flight_id_t) -1;
}

void flight_invalidate(flight_t *flight) {
    flight->id = (flight_id_t) -1;
}

void flight_free(flight_t *flight) {
    if (flight->owns_airline)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->airline);

    if (flight->owns_plane_model)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->plane_model);

    if (flight->owns_itself)
        free(flight);
}
