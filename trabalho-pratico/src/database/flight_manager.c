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
 * @file  flight_manager.c
 * @brief Implementation of methods in include/database/flight_manager.h
 *
 * #### Example
 * See [the header file's documentation](@ref flight_manager_examples).
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "database/flight_manager.h"
#include "utils/pool.h"
#include "utils/string_pool.h"

/**
 * @struct flight_manager
 * @brief  A data type that contains and manages all flights in a database.
 *
 * @var flight_manager::flights
 *     @brief Set of users in the manager.
 * @var flight_manager::strings
 *     @brief Pool for any string that may need to be stored in a flight.
 * @var flight_manager::id_flights_rel
 *     @brief Hash table for identifier -> flights mapping.
 */
struct flight_manager {
    pool_t        *flights;
    string_pool_t *strings;
    GHashTable    *id_flights_rel;
};

/** @brief Number of flights in each block of ::flight_manager::flights. */
#define FLIGHT_MANAGER_FLIGHTS_POOL_BLOCK_CAPACITY 20000

/** @brief Number of characters in each block of ::flight_manager::strings. */
#define FLIGHT_MANAGER_STRINGS_POOL_BLOCK_CAPACITY 100000

flight_manager_t *flight_manager_create(void) {
    flight_manager_t *manager = malloc(sizeof(struct flight_manager));
    if (!manager)
        return NULL;

    manager->flights = __pool_create(flight_sizeof(), FLIGHT_MANAGER_FLIGHTS_POOL_BLOCK_CAPACITY);
    if (!manager->flights) {
        free(manager);
        return NULL;
    }

    manager->strings = string_pool_create(FLIGHT_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->strings) {
        pool_free(manager->flights);
        free(manager);
        return NULL;
    }

    manager->id_flights_rel = g_hash_table_new(g_str_hash, g_str_equal);

    return manager;
}

flight_t *flight_manager_add_flight(flight_manager_t *manager, const flight_t *flight) {
    flight_t *pool_flight = pool_put_item(flight_t, manager->flights, flight);
    if (!pool_flight)
        return NULL;

    /* Copy strings to string pool */
    char *pool_airline = string_pool_put(manager->strings, flight_get_const_airline(flight));
    char *pool_plane_model =
        string_pool_put(manager->strings, flight_get_const_plane_model(flight));

    if (pool_airline && pool_plane_model) {
        flight_set_airline(pool_flight, pool_airline);
        flight_set_plane_model(pool_flight, pool_plane_model);

        size_t pool_id = flight_get_id(flight);
        if (!g_hash_table_insert(manager->id_flights_rel, &pool_id, pool_flight)) {
            fprintf(stderr,
                    "REPEATED FLIGHT ID \"%ld\". This shouldn't happen! Replacing it.\n",
                    pool_id);
            /* Do not fail and return NULL. Show must go on */
        }

        return pool_flight;
    } else {
        /* On allocation failure, it's impossible to remove anything already in a pool */
        flight_invalidate(pool_flight);
        return NULL;
    }
}

flight_t *flight_manager_get_by_id(const flight_manager_t *manager, const char *id) {
    return g_hash_table_lookup(manager->id_flights_rel, id);
}

/**
 * @struct user_manager_iter_user_data_t
 * @brief Internal data type for the `user_data` parameter in ::__user_manager_iter_callback.
 *
 * @var user_manager_iter_user_data_t::callback
 *     @brief Callback to be called for every valid user.
 * @var user_manager_iter_user_data_t::original_user_data.
 *     @brief `user_data` parameter for every ::user_manager_iter_user_data_t::callback.
 */
typedef struct {
    flight_manager_iter_callback_t callback;
    void                          *original_flight_data;
} flight_manager_iter_flight_data_t;

/**
 * @brief   Callback for every item in the user manager's pool.
 * @details Auxiliary function for ::user_manager_iter. Makes sure the target callback is only
 *          called for valid users.
 *
 * @param user_data A ::user_manager_iter_user_data_t.
 * @param item      A ::user_t in a user manager's pool.
 *
 * @return The return value of the target callback, or `0` for filtered-out items.
 */
int __flight_manager_iter_callback(void *flight_data, void *item) {
    if (flight_is_valid((flight_t *) item) == 0) {
        flight_manager_iter_flight_data_t *helper_data =
            (flight_manager_iter_flight_data_t *) flight_data;
        return helper_data->callback(helper_data->original_flight_data, (flight_t *) item);
    }

    return 0;
}

int flight_manager_iter(flight_manager_t              *manager,
                        flight_manager_iter_callback_t callback,
                        void                          *flight_data) {

    flight_manager_iter_flight_data_t helper_data = {.callback             = callback,
                                                     .original_flight_data = flight_data};
    return pool_iter(manager->flights, __flight_manager_iter_callback, &helper_data);
}

void flight_manager_free(flight_manager_t *manager) {
    pool_free(manager->flights);
    string_pool_free(manager->strings);
    g_hash_table_destroy(manager->id_flights_rel);
    free(manager);
}
