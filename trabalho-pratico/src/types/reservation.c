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
 * ### Example
 * See [the header file's documentation](@ref reservation_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "types/reservation.h"

/**
 * @struct  reservation
 * @brief   A reservation.
 * @details Some fields in the project's requirements (such as address, room details and comments)
 *          aren't put here, as they aren't required by any of the queries.
 *
 * @var reservation::user_id
 *     @brief Identifier of the user that booked a given reservation.
 * @var reservation::hotel_name
 *     @brief Name of the hotel of a given reservation.
 * @var reservation::includes_breakfast
 *     @brief Whether or not this booling includes breakfast.
 * @var reservation::begin_date
 *     @brief The beginning date of a given reservation.
 * @var reservation::end_date
 *     @brief The end date of a given reservation.
 * @var reservation::id
 *     @brief Identifier of a given reservation.
 * @var reservation::rating
 *     @brief Rating of a given reservation. It's a value between 1 and 5 (inclusive), or
 *            ::RESERVATION_NO_RATING, meaning
 * @var reservation::hotel_id
 *     @brief Identifier of the hotel of a given reservation.
 * @var reservation::hotel_stars
 *     @brief Number of stars of the hotel of a given reservation.
 * @var reservation::city_tax
 *     @brief City tax of a given reservation.
 * @var reservation::price_per_night
 *     @brief Price per night of a given reservation.
 * @var reservation::owns_itself
 *     @brief   Whether, when `free`ing this reservation, the reservation pointer should be
 *              `free`'d.
 *     @details A false value means that the reservation is allocated in a pool.
 * @var reservation::owns_user_id
 *     @brief   Whether ::reservation::user_id should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 * @var reservation::owns_hotel_name
 *     @brief   Whether ::reservation::hotel_name should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 */
struct reservation {
    char                *user_id;
    const char          *hotel_name;
    date_t               begin_date;
    date_t               end_date;
    reservation_id_t     id;
    hotel_id_t           hotel_id;
    uint16_t             price_per_night;
    uint8_t              city_tax;
    uint8_t              rating;
    uint8_t              hotel_stars;
    includes_breakfast_t includes_breakfast : 1;

    int owns_itself : 1, owns_user_id : 1, owns_hotel_name : 1;
};

reservation_t *reservation_create(pool_t *allocator) {
    reservation_t *ret =
        allocator ? pool_alloc_item(reservation_t, allocator) : malloc(sizeof(reservation_t));
    if (!ret)
        return NULL;

    ret->owns_itself  = allocator == NULL;
    ret->owns_user_id = ret->owns_hotel_name = 0; /* Don't free in first setter call */
    reservation_reset_dates(ret); /* For first comparisons to work */

    return ret;
}

reservation_t *reservation_clone(pool_t                      *allocator,
                                 string_pool_t               *user_id_allocator,
                                 string_pool_no_duplicates_t *hotel_name_allocator,
                                 const reservation_t         *reservation) {

    reservation_t *ret = reservation_create(allocator);
    if (!ret)
        return NULL;

    memcpy(ret, reservation, sizeof(reservation_t));
    ret->owns_itself  = allocator == NULL;
    ret->owns_user_id = ret->owns_hotel_name = 0; /* Don't free in first setter call */

    if (reservation_set_user_id(user_id_allocator, ret, reservation->user_id) ||
        reservation_set_hotel_name(hotel_name_allocator, ret, reservation->hotel_name)) {

        if (ret->owns_itself)
            free(ret);
        return NULL;
    }

    return ret;
}

int reservation_set_user_id(string_pool_t *allocator,
                            reservation_t *reservation,
                            const char    *user_id) {
    if (!*user_id)
        return 1;

    char *new_user_id = allocator ? string_pool_put(allocator, user_id) : strdup(user_id);
    if (!new_user_id)
        return 1;

    if (reservation->owns_user_id)
        free(reservation->user_id);
    reservation->owns_user_id = allocator == NULL;

    reservation->user_id = new_user_id;
    return 0;
}

int reservation_set_hotel_name(string_pool_no_duplicates_t *allocator,
                               reservation_t               *reservation,
                               const char                  *hotel_name) {
    if (!*hotel_name)
        return 1;

    const char *new_hotel_name =
        allocator ? string_pool_no_duplicates_put(allocator, hotel_name) : strdup(hotel_name);
    if (!new_hotel_name)
        return 1;

    if (reservation->owns_hotel_name)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) reservation->hotel_name);
    reservation->owns_hotel_name = allocator == NULL;

    reservation->hotel_name = new_hotel_name;
    return 0;
}

void reservation_set_includes_breakfast(reservation_t       *reservation,
                                        includes_breakfast_t includes_breakfast) {
    reservation->includes_breakfast = includes_breakfast;
}

int reservation_set_begin_date(reservation_t *reservation, date_t begin_date) {
    if (date_diff(begin_date, reservation->end_date) > 0)
        return 1;

    reservation->begin_date = begin_date;
    return 0;
}

int reservation_set_end_date(reservation_t *reservation, date_t end_date) {
    if (date_diff(reservation->begin_date, end_date) > 0)
        return 1;

    reservation->end_date = end_date;
    return 0;
}

void reservation_reset_dates(reservation_t *reservation) {
    reservation->begin_date = 0;
    reservation->end_date   = 0xFFFFFFFF;
}

void reservation_set_id(reservation_t *reservation, reservation_id_t id) {
    reservation->id = id;
}

int reservation_set_rating(reservation_t *reservation, unsigned int rating) {
    if (rating <= 5) {
        reservation->rating = rating;
        return 0;
    }
    return 1;
}

void reservation_set_hotel_id(reservation_t *reservation, hotel_id_t hotel_id) {
    reservation->hotel_id = hotel_id;
}

int reservation_set_hotel_stars(reservation_t *reservation, unsigned int hotel_stars) {
    if (1 <= hotel_stars && hotel_stars <= 5) {
        reservation->hotel_stars = hotel_stars;
        return 0;
    }
    return 1;
}

void reservation_set_city_tax(reservation_t *reservation, uint8_t city_tax) {
    reservation->city_tax = city_tax;
}

int reservation_set_price_per_night(reservation_t *reservation, uint16_t price_per_night) {
    if (!price_per_night)
        return 1;

    reservation->price_per_night = price_per_night;
    return 0;
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

reservation_id_t reservation_get_id(const reservation_t *reservation) {
    return reservation->id;
}

uint8_t reservation_get_rating(const reservation_t *reservation) {
    return reservation->rating;
}

hotel_id_t reservation_get_hotel_id(const reservation_t *reservation) {
    return reservation->hotel_id;
}

uint8_t reservation_get_hotel_stars(const reservation_t *reservation) {
    return reservation->hotel_stars;
}

uint8_t reservation_get_city_tax(const reservation_t *reservation) {
    return reservation->city_tax;
}

uint16_t reservation_get_price_per_night(const reservation_t *reservation) {
    return reservation->price_per_night;
}

size_t reservation_sizeof(void) {
    return sizeof(reservation_t);
}

int reservation_is_valid(const reservation_t *reservation) {
    return reservation->id == (reservation_id_t) -1;
}

void reservation_invalidate(reservation_t *reservation) {
    reservation->id = (reservation_id_t) -1;
}

double reservation_calculate_hotel_profit(const reservation_t *reservation) {
    return reservation->price_per_night * date_diff(reservation->end_date, reservation->begin_date);
}

double reservation_calculate_price(const reservation_t *reservation) {
    return reservation_calculate_hotel_profit(reservation) * (1 + 0.01 * reservation->city_tax);
}

void reservation_free(reservation_t *reservation) {
    if (reservation->owns_user_id)
        free(reservation->user_id);

    if (reservation->owns_hotel_name)
        /* Purposely remove const. We know it was allocated by this module */
        free((char *) reservation->hotel_name);

    if (reservation->owns_itself)
        free(reservation);
}
