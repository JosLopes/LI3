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
 * @file  flight.h
 * @brief Declaration of type ::flight_t.
 *
 * @details See what fields define a flight (and thus available through getters and setters) in the
 *          [struct's documentation](@ref flight).
 *
 *          Also, keep in mind that some setters can fail due to invalid values, returning a non-`0`
 *          value.
 *
 * @anchor flight_examples
 * ### Examples
 *
 * See [the examples in flight_manager.h](@ref flight_manager_examples). The callback there,
 * `iter_callback` is a great example on how to extract all data from an existing flight and print
 * it to `stdout`.
 */

#ifndef FLIGHT_H
#define FLIGHT_H

#include "types/airport_code.h"
#include "types/flight_id.h"
#include "utils/date_and_time.h"
#include "utils/pool.h"
#include "utils/string_pool_no_duplicates.h"

/** @brief A flight. */
typedef struct flight flight_t;

/**
 * @brief   Creates a new flight with uninitialized fields.
 * @details Before using this flight, set all its fields using the setters in this module.
 *
 * @param allocator Pool where to allocate the flight. The pool's `item_size` (see
 *                  ::pool_create_from_size) must be the value returned by ::flight_sizeof. Can be
 *                  `NULL`, so that malloc is used instead of a pool.
 *
 * @return The allocated flight (`NULL` on allocation failure).
 */
flight_t *flight_create(pool_t *allocator);

/**
 * @brief Creates a deep clone of a flight.
 *
 * @param allocator        Pool where to allocate the flight. The pool's `item_size` (see
 *                         ::pool_create_from_size) must be the value returned by ::flight_sizeof.
 *                         Can be `NULL`, so that malloc is used instead of a pool.
 * @param string_allocator Pool where to allocate the strings of a flight. Can be `NULL`, so that
 *                         `strdup` is used instead of a pool.
 * @param flight           Flight to be cloned.
 *
 * @return A deep-clone of @p flight (`NULL` on allocation failure).
 */
flight_t *flight_clone(pool_t                      *allocator,
                       string_pool_no_duplicates_t *string_allocator,
                       const flight_t              *flight);

/**
 * @brief Sets a flight's airline.
 *
 * @param allocator Where to copy @p airline to. Can be `NULL`, so that `strdup` is used instead of
 *                  a pool.
 * @param flight    Flight to have its airline set.
 * @param airline   Airline of the flight. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p airline or allocation failure. @p flight wasn't modified.
 */
int flight_set_airline(string_pool_no_duplicates_t *allocator,
                       flight_t                    *flight,
                       const char                  *airline);

/**
 * @brief Sets a flight's plane model.
 *
 * @param allocator   Where to copy @p airline to. Can be `NULL`, so that `strdup` is used instead
 *                    of a pool.
 * @param flight      Flight to have its plane model set.
 * @param plane_model Plane model of the flight. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p plane_model or allocation failure. @p flight wasn't modified.
 */
int flight_set_plane_model(string_pool_no_duplicates_t *allocator,
                           flight_t                    *flight,
                           const char                  *plane_model);

/**
 * @brief Sets a flight's origin airport.
 * @param flight Flight to have its origin airport set.
 * @param origin Origin airport of the flight.
 */
void flight_set_origin(flight_t *flight, airport_code_t origin);

/**
 * @brief Sets a flight's destination airport.
 * @param flight      Flight to have its destination airport set.
 * @param destination Destination airport of the flight.
 */
void flight_set_destination(flight_t *flight, airport_code_t destination);

/**
 * @brief Sets a flight's identifier.
 * @param flight Flight to have its id set.
 * @param id     Identifier of the flight.
 */
void flight_set_id(flight_t *flight, flight_id_t id);

/**
 * @brief Sets a flight's scheduled departure date.
 * @param flight                  Flight to have its scheduled departure date set.
 * @param schedule_departure_date Scheduled departure date of the flight. Must be a date before
 *                                `schedule_arrival_date`. If that date hasn't yet been initialized,
 *                                that comparsion won't be performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p schedule_departure_date doesn't come before `schedule_arrival_date`.
 *           @p flight wasn't modified.
 */
int flight_set_schedule_departure_date(flight_t *flight, date_and_time_t schedule_departure_date);

/**
 * @brief Sets a flight's scheduled arrival date.
 * @param flight                Flight to have its scheduled arrival date set.
 * @param schedule_arrival_date Scheduled arrival date of the flight. Must be a date after
 *                                `schedule_departure_date`. If that date hasn't yet been
 *                                initialized, that comparison won't be performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p schedule_arrival_date doesn't come after `schedule_departure_date`.
 *           @p flight wasn't modified.
 */
int flight_set_schedule_arrival_date(flight_t *flight, date_and_time_t schedule_arrival_date);

/**
 * @brief   Resets `schedule_departure_date` and `schedule_arrival_date` in a flight.
 * @details This is done so that the setters ::flight_set_schedule_departure_date and
 *          ::flight_set_schedule_arrival_date can work without previous dates making validity
 *          comparisons fail.
 *
 *          You must call both setters after calling this method.
 *
 * @param flight Flight to have its schedule dates reset.
 */
void flight_reset_schedule_dates(flight_t *flight);

/**
 * @brief Sets a flight's number of passengers.
 * @param flight               Flight to have its number of passengers set.
 * @param number_of_passengers Number of passengers of the flight. Must be lower than the total
 *                             number of seats. If that isn't initialized, that comparison won't be
 *                             performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p number_of_passengers is higher than the number of seats. @p flight
 *           wasn't modified.
 */
int flight_set_number_of_passengers(flight_t *flight, uint16_t number_of_passengers);

/**
 * @brief   Sets a flight's real departure date.
 * @details Because a flight doesn't store the real arrival date, you have to perform a validity
 *          check for that yourself (departure < arrival).
 *
 * @param flight              Flight to have its real departure date set.
 * @param real_departure_date Real departure date of the flight.
 */
void flight_set_real_departure_date(flight_t *flight, date_and_time_t real_departure_date);

/**
 * @brief Sets a flight's total number of seats.
 *
 * @param flight      Flight to have its total number seats set.
 * @param total_seats Number of seats on the flight.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p total_seats is lower than the number of passengers. @p flight wasn't
 *           modified.
 */
int flight_set_total_seats(flight_t *flight, uint16_t total_seats);

/**
 * @brief   Resets `number_of_passengers` and `total_seats` in a flight.
 * @details This is done so that the setters ::flight_set_number_of_passengers and
 *          ::flight_set_total_seats can work without previous values making validity comparisons
 *          fail.
 *
 *          You must call both setters after calling this method.
 *
 * @param flight Flight to have its seats and number of passengers reset.
 */
void flight_reset_seats(flight_t *flight);

/**
 * @brief  Gets a flight's airline.
 * @param  flight Flight to get the airline from.
 * @return The flight's airline.
 */
const char *flight_get_const_airline(const flight_t *flight);

/**
 * @brief  Gets a flight's plane model.
 * @param  flight Flight to get the plane model from.
 * @return The flight's plane model.
 */
const char *flight_get_const_plane_model(const flight_t *flight);

/**
 * @brief  Gets a flight's origin airport.
 * @param  flight Flight to get the origin airport from.
 * @return The flight's origin airport.
 */
airport_code_t flight_get_origin(const flight_t *flight);

/**
 * @brief  Gets a flight's destination airport.
 * @param  flight Flight to get the destination airport from.
 * @return The flight's destination airport.
 */
airport_code_t flight_get_destination(const flight_t *flight);

/**
 * @brief  Gets a flight's identifier.
 * @param  flight Flight to get the id from.
 * @return The flight's identifier.
 */
flight_id_t flight_get_id(const flight_t *flight);

/**
 * @brief  Gets a flight's scheduled departure date.
 * @param  flight Flight to get the scheduled departure date from.
 * @return The flight's scheduled departure date.
 */
date_and_time_t flight_get_schedule_departure_date(const flight_t *flight);

/**
 * @brief  Gets a flight's scheduled arrival date.
 * @param  flight Flight to get the scheduled arrival date from.
 * @return The flight's scheduled arrival date.
 */
date_and_time_t flight_get_schedule_arrival_date(const flight_t *flight);

/**
 * @brief  Gets a flight's number of passengers.
 * @param  flight Flight to get the number of passengers from.
 * @return The flight's number of passengers.
 */
uint16_t flight_get_number_of_passengers(const flight_t *flight);

/**
 * @brief  Gets a flight's real departure date.
 * @param  flight Flight to get the real departure date from.
 * @return The flight's departure date.
 */
date_and_time_t flight_get_real_departure_date(const flight_t *flight);

/**
 * @brief  Gets a flight's total number of seats.
 * @param  flight Flight to get the total number of seats from.
 * @return The flight's total number of seats.
 */
uint16_t flight_get_total_seats(const flight_t *flight);

/**
 * @brief   Gets the size of a ::flight_t in memory.
 * @details Useful for pool allocation.
 * @return  `sizeof(flight_t)`.
 */
size_t flight_sizeof(void);

/**
 * @brief   Checks if a flight in a database is valid.
 * @details Flights can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param flight Flight to be checked.
 *
 * @retval 0 Valid flight.
 * @retval 1 Invalid flight.
 */
int flight_is_valid(const flight_t *flight);

/**
 * @brief   Alters a flight in a database to make it invalid.
 * @details Flights can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param flight Flight to be invalidated.
 */
void flight_invalidate(flight_t *flight);

/**
 * @brief Frees the memory used for a given flight.
 * @param flight Flight to be deleted.
 */
void flight_free(flight_t *flight);

#endif
