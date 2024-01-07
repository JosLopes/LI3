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
 * @file    database.h
 * @brief   Collection of managers of the different entities.
 * @details Its contents are to be created with the ::dataset_loader_t.
 *
 * @anchor database_examples
 * ### Example
 *
 * This example describes how to create a ::database_t and load data to it using a
 * ::dataset_loader_t.
 *
 * ```c
 * int main(void) {
 *     database_t *database = database_create();
 *     if (!database) {
 *         fprintf(stderr, "Failed to allocate database!");
 *         return 1;
 *     }
 *
 *     if (dataset_loader_load(database, "/path/to/dataset/directory", NULL, NULL)) {
 *         fputs("Failed to open dataset to be parsed.\n", stderr);
 *         return 1;
 *     }
 *
 *     // Run your queries on database here
 *
 *     database_free(database);
 *     return 0;
 * }
 * ```
 *
 * If you don't wish to use ::dataset_loader_load, you may get the entity managers in the database
 * (by using ::database_get_users for a ::user_manager_t, for example) and interact with those
 * managers directly.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "database/flight_manager.h"
#include "database/reservation_manager.h"
#include "database/user_manager.h"

/** @brief A database is a collection of the managers of different entities. */
typedef struct database database_t;

/**
 * @brief   Instantiates a new ::database_t.
 * @details The returned value is owned by the caller and should be `free`'d with ::database_free.
 * @return  The new database, or `NULL` on failure.
 *
 * #### Example
 * See [the header file's documentation](@ref database_examples).
 */
database_t *database_create(void);

/**
 * @brief Gets the user manager in a database.
 * @param database Database to get the user manager from.
 * @returns The user manager in @p database.
 */
const user_manager_t *database_get_users(const database_t *database);

/**
 * @brief Gets the reservation manager in a database.
 * @param database Database to get the reservation manager from.
 * @returns The reservation manager in @p database.
 */
const reservation_manager_t *database_get_reservations(const database_t *database);

/**
 * @brief Gets the flight manager in a database.
 * @param database Database to get the flight manager from.
 * @returns The flight manager in @p database.
 */
const flight_manager_t *database_get_flights(const database_t *database);

/**
 * @brief Adds a user to @p database.
 *
 * @param database Database to add @p user to.
 * @param user     User to be added to @p database.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure.
 */
int database_add_user(database_t *database, const user_t *user);

/**
 * @brief Adds a reservation to @p database.
 *
 * @param database    Database to add @p reservation to.
 * @param reservation Reservation to be added to @p database.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure.
 */
int database_add_reservation(database_t *database, const reservation_t *reservation);

/**
 * @brief Adds a flight to @p database.
 *
 * @param database Database to add @p flight to.
 * @param flight   Flight to be added to @p database.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure.
 */
int database_add_flight(database_t *database, const flight_t *flight);

/**
 * @brief   Removes a flight from the database.
 * @details It's assumed that there are no users with passenger relations to @p flight.
 *
 * @param database Database to get a flight removed from.
 * @param id       Identifier of the flight to invalidate.
 *
 * @retval 0 Flight was in @p database and was invalidated.
 * @retval 1 Flight not in @p database to begin with.
 */
int database_invalidate_flight(database_t *database, flight_id_t id);

/**
 * @brief Adds a user-flight relation to the user manager.
 *
 * @param database  Database to add the relation to.
 * @param user_id   Identifier of the user to add @p flight_id to.
 * @param flight_id Identifier of the flight to be associated with @p user_id.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure or user / flight not found.
 */
int database_add_passenger(database_t *database, const char *user_id, flight_id_t flight_id);

/**
 * @brief Frees memory used by a database.
 * @param database Database whose memory is to be `free`'d.
 *
 * #### Example
 * See [the header file's documentation](@ref database_examples).
 */
void database_free(database_t *database);

#endif
