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
 * @file    flight_manager.h
 * @brief   Contains and manages all flights in a database.
 * @details Usually, a flight manager won't be created by itself, but instead by a ::database_t.
 *
 * @anchor flight_manager_examples
 * ### Examples
 *
 * In the following example, a dataset is loaded into a database. The flight manager is then
 * extracted from the database, and the program iterates over all flights.
 *
 * ```c
 * #include <stdio.h>
 *
 * #include "database/flight_manager.h"
 * #include "dataset/dataset_loader.h"
 * #include "types/flight.h"
 * #include "utils/pool.h"
 *
 * int iter_callback(void *user_data, const flight_t *flight) {
 *     (void) user_data;
 *
 *     const flight_id_t id          = flight_get_id(flight);
 *     const char       *airline     = flight_get_const_airline(flight);
 *     const char       *passport    = flight_get_const_plane_model(flight);
 *     uint16_t          total_seats = flight_get_total_seats(flight);
 *
 *     char origin[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
 *     airport_code_sprintf(origin, flight_get_origin(flight));
 *
 *     char destination[AIRPORT_CODE_SPRINTF_MIN_BUFFER_SIZE];
 *     airport_code_sprintf(destination, flight_get_destination(flight));
 *
 *     char schedule_departure_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
 *     date_and_time_sprintf(schedule_departure_date, flight_get_schedule_departure_date(flight));
 *
 *     char schedule_arrival_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
 *     date_and_time_sprintf(schedule_arrival_date, flight_get_schedule_arrival_date(flight));
 *
 *     char real_departure_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
 *     date_and_time_sprintf(real_departure_date, flight_get_real_departure_date(flight));
 *
 *     char id_str[FLIGHT_ID_SPRINTF_MIN_BUFFER_SIZE];
 *     flight_id_sprintf(id_str, id);
 *
 *     printf("--- FLIGHT ---\nid: %s\nairline: %s\nplane_model: %s\ntotal_seats: " PRIu16
 *            "\norigin: %s\ndestination: %s\nschedule_departure_date: %s\n"
 *            "schedule_arrival_date: %s\nreal_departure_date: %s\n\n",
 *            id_str,
 *            airline,
 *            passport,
 *            total_seats,
 *            origin,
 *            destination,
 *            schedule_departure_date,
 *            schedule_arrival_date,
 *            real_departure_date);
 *
 *     return 0; // You can return a value other than 0 to order iteration to stop
 * }
 *
 * int main() {
 *     database_t *database = database_create();
 *     if (!database) {
 *         fprintf(stderr, "Failed to allocate database!\n");
 *         return 1;
 *     }
 *
 *     if (dataset_loader_load(database, "/path/to/dataset/directory")) {
 *         fputs("Failed to open dataset to be parsed.\n", stderr);
 *         return 1;
 *     }
 *
 *     flight_manager_iter(database_get_flights(database), iter_callback, NULL);
 *
 *     database_free(database);
 *     return 0;
 * }
 * ```
 *
 * Another operation (other than iteration) that can be performed on a ::flight_manager_t is a
 * lookup by flight identifier (::flight_manager_get_by_id).
 *
 * If you'd rather not use a database, you could create the flight manager yourself with
 * ::flight_manager_create, add flights to it using ::flight_manager_add_flight, and free it in the
 * end with ::flight_manager_free. Just keep in mind that added flights and their associated strings
 * will be copied to memory pools.
 */

#ifndef FLIGHT_MANAGER_H
#define FLIGHT_MANAGER_H

#include "types/flight.h"

/** @brief A data type that contains and manages all flights in a database. */
typedef struct flight_manager flight_manager_t;

/**
 * @brief   Callback type for flight manager iterations.
 * @details Method called by ::flight_manager_iter for every item in a ::flight_manager_t.
 *
 * @param user_data Argument passed to ::flight_manager_iter, that is then passed to every callback,
 *                  so that this method can change the program's state.
 * @param flight    Flight in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*flight_manager_iter_callback_t)(void *user_data, const flight_t *flight);

/**
 * @brief   Instantiates a new ::flight_manager_t.
 * @details The returned value is owned by the caller and should be `free`d with
 *          ::flight_manager_free.
 * @return  The new flight manager, or `NULL` on allocation failure.
 */
flight_manager_t *flight_manager_create(void);

/**
 * @brief   Creates a deep copy of a flight manager.
 * @details Managers usually contain lots of data, possibly even gigabytes! Keep that in mind for
 *          performance and memory usage reasons.
 *
 * @param manager Manager to be copied.
 *
 * @return A copy of @p manager, that must be `free`d with ::flight_manager_free. `NULL` can also be
 *         returned, meaning an allocation failure happened.
 */
flight_manager_t *flight_manager_clone(const flight_manager_t *manager);

/**
 * @brief Adds a flight to a flight manager.
 *
 * @param manager Flight manager to add @p flight to.
 * @param flight  Flight to add to @p manager.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int flight_manager_add_flight(flight_manager_t *manager, const flight_t *flight);

/**
 * @brief Adds a number of passengers to a flight in a flight manager.
 *
 * @param manager Manager containing the flight that needs to be modified.
 * @param id      Identifier of the flight to have its number of passagers modified.
 * @param count   Number of passengers to add to the flight.
 *
 * @retval 0 Success.
 * @retval 1 Flight didn't exist or too many passengers for number of seats. Check which case
 *           applies by calling ::flight_manager_get_by_id.
 */
int flight_manager_add_passagers(flight_manager_t *manager, flight_id_t id, int count);

/**
 * @brief Gets a flight from a flight manager by its identifier.
 *
 * @param manager Flight manager to get a flight from.
 * @param id      Identifier of the flight to get.
 *
 * @return The flight with identifier @p id, or `NULL` if it does not exist.
 */
const flight_t *flight_manager_get_by_id(const flight_manager_t *manager, flight_id_t id);

/**
 * @brief   Invalidates a flight stored in a manager.
 * @details Memory can't be `free`d by deleting a flight, given the internal structure of the
 *          manager. However, the deleted flight won't appear in later lookups or iterations.
 *
 * @param manager Flight manager remove a flight from.
 * @param id      Identifier of the flight to invalidate.
 *
 * @retval 0 Flight was in the manager and was invalidated.
 * @retval 1 Flight not in the manager to begin with.
 */
int flight_manager_invalidate_by_id(flight_manager_t *manager, flight_id_t id);

/**
 * @brief Iterates through every user in a flight manager, calling a callback for each one.
 *
 * @param manager   Flight manager to iterate over.
 * @param callback  Method called for every flight in @p manager.
 * @param user_data Argument passed to @p callback.
 *
 * @return The return value of the last-called @p callback (`0` means success, another value means
 *         the iteration was stopped by a callback).
 */
int flight_manager_iter(const flight_manager_t        *manager,
                        flight_manager_iter_callback_t callback,
                        void                          *user_data);

/**
 * @brief Frees memory used by a flight manager.
 * @param manager Flight manager whose memory is to be `free`'d.
 */
void flight_manager_free(flight_manager_t *manager);

#endif
