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
 * @file     reservation.h
 * @brief    Declaration of type `reservation_t`.
 *
 * @details  In this module you can find a declaration of the type `reservation_t` as a
 *           struct reservation, as well as get and set functions, which allow's the developer
 *           access to any previously created reservation, or gives them the ability to create
 *           a new one.
 *           Reservation parameters available for the developer:
 *
 * @param user_id            Id of the reservation's user, of type `char*`.
 * @param hotel_id           Id of the reservation's hotel, of type `int` ("HTL" + int).
 * @param hotel_name         Name of the reservation's hotel, of type `char*`.
 * @param rating             Rating attributed by a user, of type `int*`
 *                           (NULL if the user didn't submit a rating).
 * @param includes_breakfast Flag that indicates if a given reservation includes
 *                           breakfast, of type `enum includes_breakfast`.
 * @param id                 Id of a given reservation, of type `size_t`.
 * @param hotel_stars        Stars assigned the reservation's hotel, of type `int`.
 * @param begin_date         Beginning date of a given reservation, of type `date_t`.
 * @param end_date           End date of a given reservation, of type `date_t`.
 * @param city_tax           Percentage of city tax (over total amount), of type `int`.
 * @param price_per_night    Price per night of a giver reservation, of type `int`.
 */
#ifndef FLIGHT_H
#define FLIGHT_H

#include <stddef.h>

#include "utils/date.h"

/**
 * @brief Enum includes_breakfast, if the value is
 *        zero (received as an int)/ no_input (no value received)/ f/ _false
 *        the reservation doesn't include breakfast, otherwise it does.
 */
enum includes_breakfast {
    zero,
    one,
    no_input,
    f,
    _false,
    t,
    _true
};

/**
 * @brief Type `reservation_t` defined as a struct reservation,
 *        stores valuable information of a given reservation (Opaque type).
 */
typedef struct reservation reservation_t;

/**
 * @brief Creates a new reservation.
 *
 * @return new_reservation, the new reservation created of type `reservation_t*`.
 */
reservation_t *create_reservation(void);

/**
 * @brief Sets the reservation's user_id, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_reservation).
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param user_id     User id of a given reservation, of type `char*`.
 */
void set_reservation_user_id(reservation_t *reservation, char *user_id);

/**
 * @brief Sets the reservation's hotel_id, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_reservation).
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param hotel_id    Hotel id of a given reservation, of type `int`.
 */
void set_reservation_hotel_id(reservation_t *reservation, int hotel_id);

/**
 * @brief Sets the reservation's hotel_name, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_reservation).
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param hotel_name  Hotel name of a given reservation, of type `char*`.
 */
void set_reservation_hotel_name(reservation_t *reservation, char *hotel_name);

/**
 * @brief Sets the reservation's rating. Be sure to free this previously allocated memory
 *        when no longer necessary (using the function free_reservation).
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param rating      Rating of a given reservation, of type `int*`.
 */
void set_reservation_rating(reservation_t *reservation, int *rating);

/**
 * @brief Sets the reservation's flag includes_breakfast.
 *
 * @param reservation        Reservation of type `reservation_t*`.
 * @param includes_breakfast Flag that indicates if a given reservation includes
 *                           breakfast, of type `char*`.
 */
void set_reservation_includes_breakfast(reservation_t *reservation,
                                        char          *includes_breakfast_string);

/**
 * @brief Sets the reservation's id.
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param id          Id of a given reservation, of type `size_t`.
 */
void set_reservation_id(reservation_t *reservation, size_t id);

/**
 * @brief Sets the reservation's hotel stars.
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param hotel_stars Stars assigned the reservation's hotel, of type `int*`.
 */
void set_reservation_hotel_stars(reservation_t *reservation, int hotel_stars);

/**
 * @brief Sets the reservation's begin date.
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param begin_date  Begin date of a given reservation, of type `date_t`.
 */
void set_reservation_begin_date(reservation_t *reservation, date_t begin_date);

/**
 * @brief Sets the reservation's end date.
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param end_date    End date of a given reservation, of type `date_t`.
 */
void set_reservation_end_date(reservation_t *reservation, date_t end_date);

/**
 * @brief Sets the reservation's city tax.
 *
 * @param reservation Reservation of type `reservation_t*`.
 * @param city_tax    Percentage of city tax (over total amount), of type `int`.
 */
void set_reservation_city_tax(reservation_t *reservation, int city_tax);

/**
 * @brief Sets the reservation' price per night.
 *
 * @param reservation     Reservation of type `reservation_t*`.
 * @param price_per_night Price per night of a given reservation, of type `int`.
 */
void set_reservation_price_per_night(reservation_t *reservation, int price_per_night);

/**
 * @brief Gets the reservation's user id.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's user_id, of type `const char*`. 
 */
const char *get_const_reservation_user_id(reservation_t *reservation);

/**
 * @brief Gets the reservation's hotel id.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's hotel_id, of type `int`. 
 */
int get_const_reservation_hotel_id(reservation_t *reservation);

/**
 * @brief Gets the reservation's hotel name.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation' hotel_name, of type `const char*`. 
 */
const char *get_const_reservation_hotel_name(reservation_t *reservation);

/**
 * @brief Gets the reservation's rating.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's rating, of type `const int*`. 
 */
const int *get_const_reservation_rating(reservation_t *reservation);

/**
 * @brief Gets the reservation's flag includes_breakfast.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's flag include_breakfast, of type `enum includes_breakfast`. 
 */
enum includes_breakfast get_reservation_includes_breakfast(reservation_t *reservation);

/**
 * @brief Gets the reservation's id.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's id, of type `size_t`. 
 */
size_t get_reservation_id(reservation_t *reservation);

/**
 * @brief Gets the reservation's hotel stars.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's hotel_stars, of type `int`. 
 */
int get_reservation_hotel_stars(reservation_t *reservation);

/**
 * @brief Gets the reservation's begin date.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's begin_date, of type `date_t`. 
 */
date_t get_reservation_begin_date(reservation_t *reservation);

/**
 * @brief Gets the reservation's end date.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's end_date, of type `date_t`. 
 */
date_t get_reservation_end_date(reservation_t *reservation);

/**
 * @brief Gets the reservation's city tax.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's city_tax, of type `int`. 
 */
int get_reservation_city_tax(reservation_t *reservation);

/**
 * @brief Gets the reservation's price per night.
 * @param reservation Reservation of type `reservation_t*`.
 * @return The reservation's price_per_night, of type `int`. 
 */
int get_reservation_price_per_night(reservation_t *reservation);

/**
 * @brief Function that frees the memory used for a given reservation,
 *        effectively deleting the reservation.
 * @param reservation Reservation of type `reservation_t*`.
 */
void free_reservation(reservation_t *reservation);

#endif
