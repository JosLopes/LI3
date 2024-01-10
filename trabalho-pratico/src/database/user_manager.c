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
 * @file  user_manager.c
 * @brief Implementation of methods in include/database/user_manager.h
 *
 * #### Example
 * See [the header file's documentation](@ref user_manager_examples).
 */

#include <glib.h>
#include <stdio.h> /* For panic purporses */
#include <stdlib.h>

#include "database/user_manager.h"
#include "utils/pool.h"
#include "utils/string_pool.h"

/**
 * @brief A structure that contains a user and its related flight and reservation history.
 *
 * @var user_and_data_t::user
 *     @brief User data.
 * @var user_and_data_t::flights
 *     @brief Flight data for ::user_and_data_t::user.
 * @var user_and_data_t::reservations
 *     @brief Reservation data for ::user_and_data_t::user.
 */
typedef struct {
    user_t                       *user;
    single_pool_id_linked_list_t *flights;
    single_pool_id_linked_list_t *reservations;
} user_and_data_t;

/**
 * @struct user_manager
 * @brief  A data type that contains and manages all users in a database.
 *
 * @var user_manager::users
 *     @brief Set of users in the manager.
 * @var user_manager::user_data
 *     @brief Set of flights and reservations associated to each user.
 * @var user_manager::ll_data
 *     @brief Nodes for linked lists.
 * @var user_manager::strings
 *     @brief Pool for any string that may need to be stored in a user.
 * @var user_manager::id_users_rel
 *     @brief Hash table for identifier -> user_data mapping.
 */
struct user_manager {
    pool_t        *users;
    pool_t        *user_data;
    pool_t        *ll_nodes;
    string_pool_t *strings;
    GHashTable    *id_users_rel;
};

/** @brief Number of users in each block of ::user_manager::users. */
#define USER_MANAGER_USERS_POOL_BLOCK_CAPACITY 20000

/** @brief Number of flight / reservation associations in each block of ::user_manager::ll_nodes. */
#define USER_MANAGER_USERS_LL_NODES_BLOCK_CAPACITY 100000

/** @brief Number of characters in each block of ::user_manager::strings. */
#define USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY 100000

user_manager_t *user_manager_create(void) {
    user_manager_t *manager = malloc(sizeof(struct user_manager));
    if (!manager)
        return NULL;

    manager->users = pool_create_from_size(user_sizeof(), USER_MANAGER_USERS_POOL_BLOCK_CAPACITY);
    if (!manager->users) {
        free(manager);
        return NULL;
    }

    manager->user_data = pool_create(user_and_data_t, USER_MANAGER_USERS_POOL_BLOCK_CAPACITY);
    if (!manager->user_data) {
        pool_free(manager->users);
        free(manager);
        return NULL;
    }

    manager->ll_nodes =
        single_pool_id_linked_list_create_pool(USER_MANAGER_USERS_LL_NODES_BLOCK_CAPACITY);
    if (!manager->ll_nodes) {
        pool_free(manager->user_data);
        pool_free(manager->users);
        free(manager);
        return NULL;
    }

    manager->strings = string_pool_create(USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->strings) {
        pool_free(manager->ll_nodes);
        pool_free(manager->user_data);
        pool_free(manager->users);
        free(manager);
        return NULL;
    }

    manager->id_users_rel = g_hash_table_new(g_str_hash, g_str_equal);

    return manager;
}

int user_manager_add_user(user_manager_t *manager, const user_t *user) {
    user_t *pool_user = user_clone(manager->users, manager->strings, user);
    if (!pool_user)
        return 1;

    user_and_data_t user_and_data = {
        .user         = pool_user,
        .flights      = single_pool_id_linked_list_create(),
        .reservations = single_pool_id_linked_list_create(),
    };

    user_and_data_t *pool_user_and_data =
        pool_put_item(user_and_data_t, manager->user_data, &user_and_data);

    /* TODO - find way of keeping const */
    if (!g_hash_table_insert(manager->id_users_rel,
                             (char *) user_get_const_id(pool_user),
                             pool_user_and_data)) {
        fprintf(stderr,
                "REPEATED USER ID \"%s\". This shouldn't happen! Replacing it.\n",
                user_get_const_id(user));
        /* Do not fail and return NULL. Show must go on */
    }

    return 0;
}

int user_manager_add_user_flight_association(user_manager_t *manager,
                                             const char     *user_id,
                                             uint64_t        flight_id) {

    user_and_data_t *data = g_hash_table_lookup(manager->id_users_rel, user_id);
    if (!data)
        return 1;

    single_pool_id_linked_list_t *tmp =
        single_pool_id_linked_list_append_beginning(manager->ll_nodes, data->flights, flight_id);
    if (!tmp)
        return 1; /* Allocation failure */
    data->flights = tmp;
    return 0;
}

int user_manager_add_user_reservation_association(user_manager_t *manager,
                                                  const char     *user_id,
                                                  uint64_t        reservation_id) {

    user_and_data_t *data = g_hash_table_lookup(manager->id_users_rel, user_id);
    if (!data)
        return 1;

    single_pool_id_linked_list_t *tmp =
        single_pool_id_linked_list_append_beginning(manager->ll_nodes,
                                                    data->reservations,
                                                    reservation_id);
    if (!tmp)
        return 1; /* Allocation failure */
    data->reservations = tmp;
    return 0;
}

const user_t *user_manager_get_by_id(const user_manager_t *manager, const char *id) {
    user_and_data_t *data = g_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->user;
}

const single_pool_id_linked_list_t *user_manager_get_flights_by_id(const user_manager_t *manager,
                                                                   const char           *id) {
    user_and_data_t *data = g_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->flights;
}

const single_pool_id_linked_list_t *
    user_manager_get_reservations_by_id(const user_manager_t *manager, const char *id) {
    user_and_data_t *data = g_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->reservations;
}

int user_manager_iter(const user_manager_t        *manager,
                      user_manager_iter_callback_t callback,
                      void                        *user_data) {

    return pool_iter(manager->users, (pool_iter_callback_t) callback, user_data);
}

void user_manager_free(user_manager_t *manager) {
    pool_free(manager->users);
    pool_free(manager->user_data);
    pool_free(manager->ll_nodes);
    string_pool_free(manager->strings);
    g_hash_table_destroy(manager->id_users_rel);
    free(manager);
}
