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
 * ### Example
 * See [the header file's documentation](@ref flight_examples).
 */
#include <stdlib.h>
#include <string.h>

#include "types/flight.h"

/**
 * @struct  flight
 * @brief   A flight.
 * @details Some fields in the project's requirements (such as real arrival date, pilot, copilot
 *          and notes) aren't put here, as they aren't required by any of the queries.
 *
 * @var flight::airline
 *     @brief Airline of a given flight.
 * @var flight::plane_model
 *     @brief Plane model of a given flight.
 * @var flight::origin
 *     @brief Origin airport of a given flight.
 * @var flight::destination
 *     @brief Destination airport of a given flight.
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
 *     @brief Total number of seats of a given flight.
 * @var flight::owns_itself
 *     @brief   Whether, when `free`ing this flight, the flight pointer should be `free`'d.
 *     @details A false value means that the flight is allocated in a pool.
 * @var flight::owns_airline
 *     @brief   Whether ::flight::airline should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 * @var flight::owns_plane_model
 *     @brief   Whether ::flight::plane_model should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 */
struct flight {
    const char     *airline;
    const char     *plane_model;
    date_and_time_t schedule_departure_date;
    date_and_time_t real_departure_date;
    date_and_time_t schedule_arrival_date;
    airport_code_t  origin;
    airport_code_t  destination;
    flight_id_t     id;
    uint16_t        number_of_passengers;
    uint16_t        total_seats;

    int owns_itself : 1, owns_airline : 1, owns_plane_model : 1;
};

flight_t *flight_create(pool_t *allocator) {
    flight_t *const ret = allocator ? pool_alloc_item(flight_t, allocator) : malloc(sizeof(flight_t));
    if (!ret)
        return NULL;

    ret->owns_itself  = allocator == NULL;
    ret->owns_airline = ret->owns_plane_model = 0; /* Don't free in first setter call */

    /* For first comparisons to work */
    flight_reset_schedule_dates(ret);
    flight_reset_seats(ret);
    return ret;
}

flight_t *flight_clone(pool_t                      *allocator,
                       string_pool_no_duplicates_t *string_allocator,
                       const flight_t              *flight) {

    flight_t *const ret = flight_create(allocator);
    if (!ret)
        return NULL;

    memcpy(ret, flight, sizeof(flight_t));
    ret->owns_itself  = allocator == NULL;
    ret->owns_airline = ret->owns_plane_model = 0; /* Don't free in first setter call */

    if (flight_set_airline(string_allocator, ret, flight->airline) ||
        flight_set_plane_model(string_allocator, ret, flight->plane_model)) {

        if (ret->owns_itself)
            free(ret);
        return NULL;
    }

    return ret;
}

int flight_set_airline(string_pool_no_duplicates_t *allocator,
                       flight_t                    *flight,
                       const char                  *airline) {
    if (!*airline)
        return 1;

    const char *const new_airline =
        allocator ? string_pool_no_duplicates_put(allocator, airline) : strdup(airline);
    if (!new_airline)
        return 1;

    if (flight->owns_airline)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->airline);
    flight->owns_airline = allocator == NULL;

    flight->airline = new_airline;
    return 0;
}

int flight_set_plane_model(string_pool_no_duplicates_t *allocator,
                           flight_t                    *flight,
                           const char                  *plane_model) {
    if (!*plane_model)
        return 1;

    const char *const new_plane_model =
        allocator ? string_pool_no_duplicates_put(allocator, plane_model) : strdup(plane_model);
    if (!new_plane_model)
        return 1;

    if (flight->owns_plane_model)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) flight->plane_model);
    flight->owns_plane_model = allocator == NULL;

    flight->plane_model = new_plane_model;
    return 0;
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

int flight_set_schedule_departure_date(flight_t *flight, date_and_time_t scheduled_departure_date) {
    if (date_and_time_diff(scheduled_departure_date, flight->schedule_arrival_date) > 0)
        return 1;

    flight->schedule_departure_date = scheduled_departure_date;
    return 0;
}

int flight_set_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date) {
    if (date_and_time_diff(flight->schedule_departure_date, schedule_arrival_date) > 0)
        return 1;

    flight->schedule_arrival_date = schedule_arrival_date;
    return 0;
}

void flight_reset_schedule_dates(flight_t *flight) {
    flight->schedule_departure_date = 0;
    flight->schedule_arrival_date   = 0xFFFFFFFFFFFFFFFF;
}

int flight_set_number_of_passengers(flight_t *flight, uint16_t number_of_passengers) {
    if (number_of_passengers >= flight->total_seats)
        return 1;

    flight->number_of_passengers = number_of_passengers;
    return 0;
}

void flight_set_real_departure_date(flight_t *flight, date_and_time_t real_departure_date) {
    flight->real_departure_date = real_departure_date;
}

int flight_set_total_seats(flight_t *flight, uint16_t total_seats) {
    if (total_seats < flight->number_of_passengers)
        return 1;

    flight->total_seats = total_seats;
    return 0;
}

void flight_reset_seats(flight_t *flight) {
    flight->number_of_passengers = 0;
    flight->total_seats          = 0xFFFF;
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

uint16_t flight_get_number_of_passengers(const flight_t *flight) {
    return flight->number_of_passengers;
}

date_and_time_t flight_get_real_departure_date(const flight_t *flight) {
    return flight->real_departure_date;
}

uint16_t flight_get_total_seats(const flight_t *flight) {
    return flight->total_seats;
}

size_t flight_sizeof(void) {
    return sizeof(flight_t);
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
