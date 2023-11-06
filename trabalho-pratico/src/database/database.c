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
 */
struct database {
    user_manager_t   *users;
    flight_manager_t *flights;
};

database_t *database_create(void) {
    database_t *database = malloc(sizeof(struct database));
    if (!database)
        return NULL;

    database->users   = user_manager_create();
    database->flights = flight_manager_create();
    return database;
}

user_manager_t *database_get_users(const database_t *database) {
    return database->users;
}

flight_manager_t *database_get_flights(const database_t *database) {
    return database->flights;
}

void database_free(database_t *database) {
    user_manager_free(database->users);
    free(database);
}
