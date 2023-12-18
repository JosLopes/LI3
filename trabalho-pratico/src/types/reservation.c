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
 * @file  reservation.c
 * @brief Implementation of methods in include/types/reservation.h
 *
 * #### Example
 * See [the header file's documentation](@ref reservation_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "types/reservation.h"

/**
 * @struct reservation
 * @brief Represents a reservation in the datasets.
 * @details NOTE: some fields in the project's requirements (such as address, room details and
 *          comments) aren't put here, as they won't be required by any database query.
 *
 * @var reservation::user_id
 *     @brief User identifier of a given reservation.
 * @var reservation::hotel_name
 *     @brief Hotel name of a given reservation.
 * @var reservation::includes_breakfast
 *     @brief Flag identifying the inclusion of breakfast of a given reservation.
 * @var reservation::begin_date
 *     @brief The beginning date of a given reservation.
 * @var reservation::end_date
 *     @brief End date of a given reservation.
 * @var reservation::id
 *     @brief Identifier of a given reservation.
 * @var reservation::rating
 *     @brief Rating (equals `-1` if it wasn't given) of a given reservation.
 * @var reservation::hotel_id
 *     @brief Hotel identifier of a given reservation.
 * @var reservation::hotel_stars
 *     @brief Hotel stars of a given reservation.
 * @var reservation::city_tax
 *     @brief City tax of a given reservation.
 * @var reservation::price_per_night
 *     @brief Price per night of a given reservation.
 */
struct reservation {
    const char          *user_id;
    const char          *hotel_name;
    includes_breakfast_t includes_breakfast;
    date_t               begin_date;
    date_t               end_date;
    size_t               id;
    int                  rating;
    int                  hotel_id;
    int                  hotel_stars;
    int                  city_tax;
    int                  price_per_night;
};

reservation_t *reservation_create(void) {
    return malloc(sizeof(struct reservation));
}

void reservation_set_user_id(reservation_t *reservation, const char *user_id) {
    reservation->user_id = user_id;
}

void reservation_set_hotel_name(reservation_t *reservation, const char *hotel_name) {
    reservation->hotel_name = hotel_name;
}

void reservation_set_includes_breakfast(reservation_t       *reservation,
                                        includes_breakfast_t includes_breakfast) {
    reservation->includes_breakfast = includes_breakfast;
}

void reservation_set_begin_date(reservation_t *reservation, date_t begin_date) {
    reservation->begin_date = begin_date;
}

void reservation_set_end_date(reservation_t *reservation, date_t end_date) {
    reservation->end_date = end_date;
}

void reservation_set_id(reservation_t *reservation, size_t id) {
    reservation->id = id;
}

void reservation_set_rating(reservation_t *reservation, int rating) {
    reservation->rating = rating;
}

void reservation_set_hotel_id(reservation_t *reservation, int hotel_id) {
    reservation->hotel_id = hotel_id;
}

void reservation_set_hotel_stars(reservation_t *reservation, int hotel_stars) {
    reservation->hotel_stars = hotel_stars;
}

void reservation_set_city_tax(reservation_t *reservation, int city_tax) {
    reservation->city_tax = city_tax;
}

void reservation_set_price_per_night(reservation_t *reservation, int price_per_night) {
    reservation->price_per_night = price_per_night;
}

const char *reservation_get_const_user_id(const reservation_t *reservation) {
    return reservation->user_id;
}

const char *reservation_get_const_hotel_name(const reservation_t *reservation) {
    return reservation->hotel_name;
}

includes_breakfast_t reservation_get_includes_breakfast(const reservation_t *reservation) {
    return reservation->includes_breakfast;
}

date_t reservation_get_begin_date(const reservation_t *reservation) {
    return reservation->begin_date;
}

date_t reservation_get_end_date(const reservation_t *reservation) {
    return reservation->end_date;
}

size_t reservation_get_id(const reservation_t *reservation) {
    return reservation->id;
}

int reservation_get_rating(const reservation_t *reservation) {
    return reservation->rating;
}

int reservation_get_hotel_id(const reservation_t *reservation) {
    return reservation->hotel_id;
}

int reservation_get_hotel_stars(const reservation_t *reservation) {
    return reservation->hotel_stars;
}

int reservation_get_city_tax(const reservation_t *reservation) {
    return reservation->city_tax;
}

int reservation_get_price_per_night(const reservation_t *reservation) {
    return reservation->price_per_night;
}

void reservation_free(reservation_t *reservation) {
    free(reservation);
}

size_t reservation_sizeof(void) {
    return sizeof(struct reservation);
}

int reservation_is_valid(const reservation_t *reservation) {
    return reservation->id == (size_t) -1;
}

void reservation_invalidate(reservation_t *reservation) {
    reservation->id = -1;
}
