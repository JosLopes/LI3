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
 * @file  reservation.h
 * @brief Declaration of type ::reservation_t.
 *
 * @details In this module you can find a declaration of the type `reservation_t` as a struct
 *          reservation, as well as getter and setter functions, which allow's the developer
 *          access to any previously created reservation, or gives them the ability to create a
 *          new reservation.
 *
 *          You can see what fields define a reservation (and thus available through getters and
 *          setters) in the [struct's documentation](@ref reservation).
 *
 * @anchor reservation_examples
 * ### Examples
 *
 * See [the examples in reservation_manager.h](@ref reservation_manager_examples). The callback
 * there, `iter_callback` is a great example on how to extract all data from an existing
 * reservation and print it to `stdout`.
 */

#ifndef RESERVATION_H
#define RESERVATION_H

#include "types/hotel_id.h"
#include "types/includes_breakfast.h"
#include "types/reservation_id.h"
#include "utils/date.h"
#include "utils/pool.h"
#include "utils/string_pool_no_duplicates.h"

/** @brief Value of a reservation's rating when it's not specified. */
#define RESERVATION_NO_RATING 0

/**
 * @brief Type `reservation_t` defined as a struct reservation, that stores valuable information of
 *        a given reservation.
 * @details It's an opaque type.
 */
typedef struct reservation reservation_t;

/**
 * @brief Creates a new reservation with uninitialized fields.
 *
 * @param allocator Pool where to allocate the reservation. Its element size must be the value
 *                  returned by ::reservation_sizeof. Can be `NULL`, so that malloc is used.
 *
 * @return A allocated reservation (`NULL` on allocation failure).
 */
reservation_t *reservation_create(pool_t *allocator);

/**
 * @brief Creates a deep clone of a reservation.
 *
 * @param allocator            Pool where to allocate the reservation. Its element size must be the
 *                             value returned by ::reservation_sizeof. Can be `NULL`, so that malloc
 *                             is used.
 * @param user_id_allocator    Pool where to allocate the user identifier in a reservation. Can be
 *                             `NULL`, so that `strdup` is used.
 * @param hotel_name_allocator Pool where to allocate the hotel name in a reservation. Can be
 *                             `NULL`, so that `strdup` is used.
 * @param reservation          Reservation to be cloned.
 *
 * @return A deep-clone of @p reservation.
 */
reservation_t *reservation_clone(pool_t                      *allocator,
                                 string_pool_t               *user_id_allocator,
                                 string_pool_no_duplicates_t *hotel_name_allocator,
                                 const reservation_t         *reservation);

/**
 * @brief Sets the reservation's user identifier.
 *
 * @param allocator   Where to copy @p user_id to. Can be `NULL`, so that `strdup` is used.
 * @param reservation Reservation to have its user identifier set.
 * @param user_id     User identifier of the reservation.
 */
void reservation_set_user_id(string_pool_t *allocator,
                             reservation_t *reservation,
                             const char    *user_id);

/**
 * @brief Sets the reservation's hotel name.
 *
 * @param allocator   Where to copy @p hotel_name to. Can be `NULL`, so that `strdup` is used.
 * @param reservation Reservation to have its hotel name set.
 * @param hotel_name  Hotel name of the reservation.
 */
void reservation_set_hotel_name(string_pool_no_duplicates_t *allocator,
                                reservation_t               *reservation,
                                const char                  *hotel_name);

/**
 * @brief Sets the reservation's inclusion of breakfast.
 * @param reservation        Reservation to have its inclusion of breakfast set.
 * @param includes_breakfast A flag that gives information on the inclusion of breakfast of
 *                           a reservation.
 */
void reservation_set_includes_breakfast(reservation_t       *reservation,
                                        includes_breakfast_t includes_breakfast);

/**
 * @brief Sets the reservation's beginning date.
 * @param reservation Reservation to have its beginning date set.
 * @param begin_date  Beginning date of the reservation.
 */
void reservation_set_begin_date(reservation_t *reservation, date_t begin_date);

/**
 * @brief Sets the reservation's end date.
 * @param reservation Reservation to have its end date set.
 * @param end_date    End date of the reservation.
 */
void reservation_set_end_date(reservation_t *reservation, date_t end_date);

/**
 * @brief Sets the reservation's identifier.
 * @param reservation Reservation to have its identifier set.
 * @param id          Identifier of the reservation.
 */
void reservation_set_id(reservation_t *reservation, reservation_id_t id);

/**
 * @brief Sets the reservation's rating.
 * @param reservation Reservation to have its rating set.
 * @param rating      Rating of the reservation. ::RESERVATION_NO_RATING means no rating was
 *                    provided.
 */
void reservation_set_rating(reservation_t *reservation, uint8_t rating);

/**
 * @brief Sets the reservation's hotel identifier.
 * @param reservation Reservation to have its hotel identifier set.
 * @param hotel_id    Hotel identifier of the reservation.
 */
void reservation_set_hotel_id(reservation_t *reservation, hotel_id_t hotel_id);

/**
 * @brief Sets the reservation's hotel stars.
 * @param reservation Reservation to have its hotel stars set.
 * @param hotel_stars Hotel stars of the reservation.
 */
void reservation_set_hotel_stars(reservation_t *reservation, uint8_t hotel_stars);

/**
 * @brief Sets the reservation's city tax.
 * @param reservation Reservation to have its city tax set.
 * @param city_tax    City tax of the reservation.
 */
void reservation_set_city_tax(reservation_t *reservation, uint8_t city_tax);

/**
 * @brief Sets the reservation's price per night.
 * @param reservation     Reservation to have its price per night set.
 * @param price_per_night Price per night of the reservation.
 */
void reservation_set_price_per_night(reservation_t *reservation, uint16_t price_per_night);

/**
 * @brief  Gets the reservation's user identifier.
 * @param  reservation Reservation to get the user identifier from.
 * @return The reservation's user identifier, with modifications not allowed.
 */
const char *reservation_get_const_user_id(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's hotel name.
 * @param  reservation Reservation to get the hotel name from.
 * @return The reservation's hotel name, with modifications not allowed.
 */
const char *reservation_get_const_hotel_name(const reservation_t *reservation);

/**
 * @brief  Gets information on the inclusion of breakfast of the reservation.
 * @param  reservation Reservation to get the `includes_breakfast` flag from.
 * @return The reservation's flag `includes_breakfast`.
 */
includes_breakfast_t reservation_get_includes_breakfast(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's beginning date.
 * @param  reservation Reservation to get the beginning date from.
 * @return The reservation's beginning date.
 */
date_t reservation_get_begin_date(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's end date.
 * @param  reservation Reservation to get the end date from.
 * @return The reservation's end date.
 */
date_t reservation_get_end_date(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's identifier.
 * @param  reservation Reservation to get the identifier from.
 * @return The reservation's identifier.
 */
reservation_id_t reservation_get_id(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's rating.
 * @param  reservation Reservation to get the rating from.
 * @return The reservation's rating. ::RESERVATION_NO_RATING means no rating was provided.
 */
uint8_t reservation_get_rating(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's hotel identifier.
 * @param  reservation Reservation to get the hotel identifier from.
 * @return The reservation's hotel identifier.
 */
hotel_id_t reservation_get_hotel_id(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's hotel stars.
 * @param  reservation Reservation to get the hotel stars from.
 * @return The reservation's hotel stars.
 */
uint8_t reservation_get_hotel_stars(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's city tax.
 * @param  reservation Reservation to get the city tax from.
 * @return The reservation's city tax.
 */
uint8_t reservation_get_city_tax(const reservation_t *reservation);

/**
 * @brief  Gets the reservation's price per night.
 * @param  reservation Reservation to get the price per night from.
 * @return The reservation's price per night.
 */
uint16_t reservation_get_price_per_night(const reservation_t *reservation);

/**
 * @brief   Gets the size of a ::reservation_t in memory.
 * @details Useful for pool allocation.
 * @return  `sizeof(reservation_t)`.
 */
size_t reservation_sizeof(void);

/**
 * @brief Checks if a reservation is valid.
 *
 * @param reservation Reservation to have its validity checked.
 *
 * @retval 0 Valid reservation.
 * @retval 1 Invalid reservation.
 */
int reservation_is_valid(const reservation_t *reservation);

/**
 * @brief   Alters a reservation in a database to make it invalid.
 * @details This will get rid of the reservation's original identifier, giving it the value "-1". If
 *          you're not using pool storage, you must free it before using this method.
 *
 * @param reservation Reservation to be modified.
 */
void reservation_invalidate(reservation_t *reservation);

/**
 * @brief  Calculates the profit a hotel accrued from @p reservation.
 * @param  reservation Reservation to use to calculate hotel profit.
 * @return The profit a hotel accrued from @p reservation.
 */
double reservation_calculate_hotel_profit(const reservation_t *reservation);

/**
 * @brief  Calculates the price a ::user_t payed for @p reservation.
 * @param  reservation Reservation to use to calculate user price.
 * @return The price a ::user_t payed for @p reservation.
 */
double reservation_calculate_price(const reservation_t *reservation);

/**
 * @brief             Frees the memory used for a given reservation.
 * @details           All strings inside the reservation won't be freed, as they're not owned by the
 *                    reservation.
 * @param reservation Reservation to be deleted.
 */
void reservation_free(reservation_t *reservation);

#endif
