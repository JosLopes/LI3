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
 * @file  database.c
 * @brief Implementation of methods in include/database/database.h
 *
 * #### Examples
 * See [the header file's documentation](@ref database_examples).
 */

#include <stdlib.h>

#include "database/database.h"

/**
 * @struct database
 * @brief  A collection of the managers of different entities.
 *
 * @var database::users
 *     @brief All users and user relationships.
 * @var database::reservations
 *     @brief All reservations and reservations relationships.
 * @var database::flights
 *     @brief All flights and flight relationships.
 */
struct database {
    user_manager_t        *users;
    reservation_manager_t *reservations;
    flight_manager_t      *flights;
};

database_t *database_create(void) {
    database_t *database = malloc(sizeof(struct database));
    if (!database)
        return NULL;

    database->users        = user_manager_create();
    database->reservations = reservation_manager_create();
    database->flights      = flight_manager_create();
    return database;
}

const user_manager_t *database_get_users(const database_t *database) {
    return database->users;
}

const reservation_manager_t *database_get_reservations(const database_t *database) {
    return database->reservations;
}

const flight_manager_t *database_get_flights(const database_t *database) {
    return database->flights;
}

int database_add_user(database_t *database, const user_t *user) {
    return user_manager_add_user(database->users, user);
}

int database_add_reservation(database_t *database, const reservation_t *reservation) {
    if (reservation_manager_add_reservation(database->reservations, reservation))
        return 1;
    return user_manager_add_user_reservation_association(database->users,
                                                         reservation_get_const_user_id(reservation),
                                                         reservation_get_id(reservation));
}

int database_add_flight(database_t *database, const flight_t *flight) {
    return flight_manager_add_flight(database->flights, flight);
}

int database_invalidate_flight(database_t *database, flight_id_t id) {
    return flight_manager_invalidate_by_id(database->flights, id);
}

int database_add_passenger(database_t *database, const char *user_id, flight_id_t flight_id) {
    if (flight_manager_add_passagers(database->flights, flight_id, 1))
        return 1;

    if (user_manager_add_user_flight_association(database->users, user_id, flight_id)) {
        flight_manager_add_passagers(database->flights, flight_id, -1); /* Revert +1 passenger */
        return 1;
    }

    return 0;
}

void database_free(database_t *database) {
    user_manager_free(database->users);
    reservation_manager_free(database->reservations);
    flight_manager_free(database->flights);
    free(database);
}
