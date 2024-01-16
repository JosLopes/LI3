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
 * @details See what fields define a reservation (and thus available through getters and setters) in
 *          the [struct's documentation](@ref reservation).
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
#include "utils/string_pool.h"
#include "utils/string_pool_no_duplicates.h"

/** @brief Value of a reservation's rating when it's not specified. */
#define RESERVATION_NO_RATING 0

/** @brief A reservation. */
typedef struct reservation reservation_t;

/**
 * @brief Creates a new reservation with uninitialized fields.
 *
 * @param allocator Pool where to allocate the reservation. The pool's `item_size` (see
 *                  ::pool_create_from_size) must be the value returned by ::reservation_sizeof. Can
 *                  be `NULL`, so that malloc is used instead of a pool.
 *
 * @return The allocated reservation (`NULL` on allocation failure).
 */
reservation_t *reservation_create(pool_t *allocator);

/**
 * @brief   Creates a deep clone of a reservation.
 * @details Before using this reservation, set all its fields using the setters in this module.
 *
 * @param allocator            Pool where to allocate the reservation. Its element size must be the
 *                             value returned by ::reservation_sizeof. Can be `NULL`, so that malloc
 *                             is used instead of a pool.
 * @param user_id_allocator    Pool where to allocate the user identifier in a reservation. Can be
 *                             `NULL`, so that `strdup` is used instead of a pool.
 * @param hotel_name_allocator Pool where to allocate the hotel name in a reservation. Can be
 *                             `NULL`, so that `strdup` is used instead of a pool.
 * @param reservation          Reservation to be cloned.
 *
 * @return A deep-clone of @p reservation (`NULL` on allocation failure).
 */
reservation_t *reservation_clone(pool_t                      *allocator,
                                 string_pool_t               *user_id_allocator,
                                 string_pool_no_duplicates_t *hotel_name_allocator,
                                 const reservation_t         *reservation);

/**
 * @brief Sets a the identifier of the user that booked a reservation.
 *
 * @param allocator   Where to copy @p user_id to. Can be `NULL`, so that `strdup` is used instead
 *                    of a pool.
 * @param reservation Reservation to have its user identifier set.
 * @param user_id     Identifier of the user that booked a reservation. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p user_id or allocation failure. @p reservation wasn't modified.
 */
int reservation_set_user_id(string_pool_t *allocator,
                            reservation_t *reservation,
                            const char    *user_id);

/**
 * @brief Sets the name of the hotel in a reservation.
 *
 * @param allocator   Where to copy @p hotel_name to. Can be `NULL`, so that `strdup` is used
 *                    instead of a pool.
 * @param reservation Reservation to have its hotel name set.
 * @param hotel_name  Hotel name of the reservation. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p hotel_name or allocation failure. @p reservation wasn't
 *           modified.
 */
int reservation_set_hotel_name(string_pool_no_duplicates_t *allocator,
                               reservation_t               *reservation,
                               const char                  *hotel_name);

/**
 * @brief Sets whether a reservation includes breakfast.
 * @param reservation        Reservation to have its "includes breakfast" field set.
 * @param includes_breakfast Whether @p reservation includes breakfast.
 */
void reservation_set_includes_breakfast(reservation_t       *reservation,
                                        includes_breakfast_t includes_breakfast);

/**
 * @brief Sets a reservation's beginning date.
 *
 * @param reservation Reservation to have its beginning date set.
 * @param begin_date  Beginning date of the reservation. Must be before the reservation's end date.
 *                    If that date hasn't yet been initialized, that comparsion won't be performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p begin_date doesn't come before `end_date`. @p reservation wasn't
 *           modified.
 */
int reservation_set_begin_date(reservation_t *reservation, date_t begin_date);

/**
 * @brief Sets a reservation's end date.
 *
 * @param reservation Reservation to have its end date set.
 * @param end_date    End date of the reservation. Must be after the reservation's begin date.
 *                    If that date hasn't yet been initialized, that comparsion won't be performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p end_date doesn't come after `begin_date`. @p reservation wasn't
 *           modified.
 */
int reservation_set_end_date(reservation_t *reservation, date_t end_date);

/**
 * @brief   Resets `begin_date` and `end_date` in a reservation.
 * @details This is done so that the setters ::reservation_set_begin_date and
 *          ::reservation_set_begin_date can work without previous dates making validity
 *          comparisons fail.
 *
 *          You must call both setters after calling this method.
 *
 * @param reservation Reservation to have its dates reset.
 */
void reservation_reset_dates(reservation_t *reservation);

/**
 * @brief Sets a reservation's identifier.
 * @param reservation Reservation to have its identifier set.
 * @param id          Identifier of the reservation.
 */
void reservation_set_id(reservation_t *reservation, reservation_id_t id);

/**
 * @brief Sets a reservation's rating.
 *
 * @param reservation Reservation to have its rating set.
 * @param rating      Rating of the reservation, a number between 1 and 5 (inclusive), or
 *                    ::RESERVATION_NO_RATING for no available rating information.
 *
 * @retval 0 Success (value within range).
 * @retval 1 Failure (value out of range). @p reservation wasn't modified.
 */
int reservation_set_rating(reservation_t *reservation, unsigned int rating);

/**
 * @brief Sets the identifier of the hotel in a reservation.
 * @param reservation Reservation to have its hotel identifier set.
 * @param hotel_id    Identifier of the hotel in the reservation.
 */
void reservation_set_hotel_id(reservation_t *reservation, hotel_id_t hotel_id);

/**
 * @brief Sets the number of hotel stars in a reservation.
 *
 * @param reservation Reservation to have its hotel stars set.
 * @param hotel_stars Hotel stars of the reservation. Must be a number between 1 and 5 (inclusive).
 *
 * @retval 0 Success (value within range).
 * @retval 1 Failure (value out of range). @p reservation wasn't modified.
 */
int reservation_set_hotel_stars(reservation_t *reservation, unsigned int hotel_stars);

/**
 * @brief Sets a reservation's city tax.
 * @param reservation Reservation to have its city tax set.
 * @param city_tax    City tax of the reservation.
 */
void reservation_set_city_tax(reservation_t *reservation, uint8_t city_tax);

/**
 * @brief Sets a reservation's price per night.
 * @param reservation     Reservation to have its price per night set.
 * @param price_per_night Price per night of the reservation. Musn't be `0`.
 *
 * @retval 0 Success.
 * @retval 1 Failure (@p price_per_night was `0`). @p reservation wasn't modified.
 */
int reservation_set_price_per_night(reservation_t *reservation, uint16_t price_per_night);

/**
 * @brief  Gets a reservation's user identifier.
 * @param  reservation Reservation to get the user identifier from.
 * @return The reservation's user identifier.
 */
const char *reservation_get_const_user_id(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's hotel name.
 * @param  reservation Reservation to get the hotel name from.
 * @return The reservation's hotel name.
 */
const char *reservation_get_const_hotel_name(const reservation_t *reservation);

/**
 * @brief  Gets whether or not a reservation includes breakfast.
 * @param  reservation Reservation to get the `includes_breakfast` flag from.
 * @return Whether or not @p reservation includes breakfast.
 */
includes_breakfast_t reservation_get_includes_breakfast(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's beginning date.
 * @param  reservation Reservation to get the beginning date from.
 * @return The reservation's beginning date.
 */
date_t reservation_get_begin_date(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's end date.
 * @param  reservation Reservation to get the end date from.
 * @return The reservation's end date.
 */
date_t reservation_get_end_date(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's identifier.
 * @param  reservation Reservation to get the identifier from.
 * @return The reservation's identifier.
 */
reservation_id_t reservation_get_id(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's rating.
 * @param  reservation Reservation to get the rating from.
 * @return The reservation's rating. ::RESERVATION_NO_RATING means no rating was provided.
 */
uint8_t reservation_get_rating(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's hotel identifier.
 * @param  reservation Reservation to get the hotel identifier from.
 * @return The reservation's hotel identifier.
 */
hotel_id_t reservation_get_hotel_id(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's hotel stars.
 * @param  reservation Reservation to get the hotel stars from.
 * @return The reservation's hotel stars.
 */
uint8_t reservation_get_hotel_stars(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's city tax.
 * @param  reservation Reservation to get the city tax from.
 * @return The reservation's city tax.
 */
uint8_t reservation_get_city_tax(const reservation_t *reservation);

/**
 * @brief  Gets a reservation's price per night.
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
 * @brief   Checks if a reservation in a database is valid.
 * @details Reservations can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param reservation Reservation to have its validity checked.
 *
 * @retval 0 Valid reservation.
 * @retval 1 Invalid reservation.
 */
int reservation_is_valid(const reservation_t *reservation);

/**
 * @brief   Alters a reservation in a database to make it invalid.
 * @details Reservations can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param reservation Reservation to be modified.
 */
void reservation_invalidate(reservation_t *reservation);

/**
 * @brief  Calculates the profit a hotel accrued from @p reservation.
 * @param  reservation Reservation used to calculate hotel profit.
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
 * @brief Frees the memory used for a given reservation.
 * @param reservation Reservation to be deleted.
 */
void reservation_free(reservation_t *reservation);

#endif
