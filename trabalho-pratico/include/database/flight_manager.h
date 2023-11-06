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

#ifndef FLIGHT_MANAGER_H
#define FLIGHT_MANAGER_H

#include "types/flight.h"

/**
 * @brief A data type that contains and manages all flights in a database.
 */
typedef struct flight_manager flight_manager_t;

/**
 * @brief   Callback type for flight manager iterations.
 * @details Method called by ::flight_manager_iter for every item in a ::flight_manager_t.
 *
 * @param flight_data Argument passed to ::flight_manager_iter that is passed to every callback, so
 *                    that this method can change the program's state.
 * @param flight      Flight in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*flight_manager_iter_callback_t)(void *flight_data, flight_t *flight);

/**
 * @brief   Creates a new flight manager.
 * @details A flight manager is a data structure that contains and manages all flights in a
 *          database.
 *
 * @return A new flight manager, or `NULL` if there was not enough memory.
 */
flight_manager_t *flight_manager_create(void);

/**
 * @brief Adds a flight to a flight manager.
 *
 * @param manager Flight manager to add @p flight to.
 * @param flight  Flight to add to @p manager.
 * 
 * @return @p flight if it was successfully added, or `NULL` if there was not enough memory.
 */
flight_t *flight_manager_add_flight(flight_manager_t *manager, const flight_t *flight);

/**
 * @brief   Gets a flight from a flight manager by its identifier.
 *
 * @param manager Flight manager to get a flight from.
 * @param id      Identifier of the flight to get.
 *
 * @return The flight with identifier @p id, or `NULL` if it does not exist.
 */
flight_t *flight_manager_get_by_id(const flight_manager_t *manager, const char *id);

/**
 * @brief   Iterates over all flights in a flight manager.
 * @details Calls @p callback for every flight in @p manager.
 *
 * @param manager      Flight manager to iterate over.
 * @param callback     Method called for every flight in @p manager.
 * @param flight_data  Argument passed to @p callback.
 *
 * @return `0` if iteration was successful, or any other value if @p callback returned a non-zero
 *         value.
 */
int flight_manager_iter(flight_manager_t              *manager,
                        flight_manager_iter_callback_t callback,
                        void                          *flight_data);

/**
 * @brief Frees memory used by a flight manager.
 * @param manager Flight manager whose memory is to be `free`'d.
 */
void flight_manager_free(flight_manager_t *manager);

#endif
