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
 * ### Example
 * See [the header file's documentation](@ref user_manager_examples).
 */

#include <stdio.h> /* For panic purporses */
#include <stdlib.h>

#include "database/user_manager.h"
#include "utils/glib/GConstKeyHashTable.h"

/**
 * @struct user_manager_user_and_data_t
 * @brief  A structure that contains a user and its related flight and reservation history.
 *
 * @var user_manager_user_and_data_t::user
 *     @brief User data.
 * @var user_manager_user_and_data_t::flights
 *     @brief Flight data for ::user_manager_user_and_data_t::user.
 * @var user_manager_user_and_data_t::reservations
 *     @brief Reservation data for ::user_manager_user_and_data_t::user.
 */
typedef struct {
    const user_t *const           user;
    single_pool_id_linked_list_t *flights;
    single_pool_id_linked_list_t *reservations;
} user_manager_user_and_data_t;

/**
 * @struct user_manager
 * @brief  A data type that contains and manages all users in a database.
 *
 * @var user_manager::users
 *     @brief Allocator for users (::user_t) in the manager.
 * @var user_manager::user_data
 *     @brief Allocator for user data (::user_manager_user_and_data_t) in the manager.
 * @var user_manager::ll_nodes
 *     @brief Allocator for linked list nodes.
 * @var user_manager::strings
 *     @brief Allocator for strings stored in users.
 * @var user_manager::id_users_rel
 *     @brief Hash table for user identifier (`const char *`) -> user_manager_user_and_data_t
 *            mapping.
 */
struct user_manager {
    pool_t             *users;
    pool_t             *user_data;
    pool_t             *ll_nodes;
    string_pool_t      *strings;
    GConstKeyHashTable *id_users_rel;
};

/** @brief Number of users in each block of ::user_manager::users and ::user_manager::user_data. */
#define USER_MANAGER_USERS_POOL_BLOCK_CAPACITY 20000

/** @brief Number of flight / reservation associations in each block of ::user_manager::ll_nodes. */
#define USER_MANAGER_USERS_LL_NODES_BLOCK_CAPACITY 100000

/** @brief Number of characters in each block of ::user_manager::strings. */
#define USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY 100000

user_manager_t *user_manager_create(void) {
    user_manager_t *const manager = malloc(sizeof(user_manager_t));
    if (!manager)
        goto DEFER_1;

    manager->users = pool_create_from_size(user_sizeof(), USER_MANAGER_USERS_POOL_BLOCK_CAPACITY);
    if (!manager->users)
        goto DEFER_2;

    manager->user_data =
        pool_create(user_manager_user_and_data_t, USER_MANAGER_USERS_POOL_BLOCK_CAPACITY);
    if (!manager->user_data)
        goto DEFER_3;

    manager->ll_nodes =
        single_pool_id_linked_list_create_pool(USER_MANAGER_USERS_LL_NODES_BLOCK_CAPACITY);
    if (!manager->ll_nodes)
        goto DEFER_4;

    manager->strings = string_pool_create(USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->strings)
        goto DEFER_5;

    manager->id_users_rel = g_const_key_hash_table_new(g_str_hash, g_str_equal);
    return manager;

DEFER_5:
    pool_free(manager->ll_nodes);
DEFER_4:
    pool_free(manager->user_data);
DEFER_3:
    pool_free(manager->users);
DEFER_2:
    free(manager);
DEFER_1:
    return NULL;
}

/**
 * @brief Method called for every ::user_manager_user_and_data_t.
 *
 * @param clone_data     Pointer to the clone being generated.
 * @param user_data_data Pointer to the ::user_manager_user_and_data_t being copied.
 *
 * @retval 0 Success.
 * @retval 1 Allocation failure.
 */
int __user_manager_clone_foreach_user_data(void *clone_data, const void *user_data_data) {
    const user_manager_t *const               clone     = clone_data;
    const user_manager_user_and_data_t *const user_data = user_data_data;

    user_t *const new_user = user_clone(clone->users, clone->strings, user_data->user);
    if (!new_user)
        goto DEFER_1;

    single_pool_id_linked_list_t *const new_flights =
        single_pool_id_linked_list_clone(clone->ll_nodes, user_data->flights);
    if (user_data->flights && !new_flights)
        goto DEFER_2;

    single_pool_id_linked_list_t *const new_reservations =
        single_pool_id_linked_list_clone(clone->ll_nodes, user_data->reservations);
    if (user_data->reservations && !new_reservations)
        goto DEFER_2;

    const user_manager_user_and_data_t new_data = {.user         = new_user,
                                                   .flights      = new_flights,
                                                   .reservations = new_reservations};

    user_manager_user_and_data_t *const pool_user_and_data =
        pool_put_item(user_manager_user_and_data_t, clone->user_data, &new_data);
    if (!pool_user_and_data)
        goto DEFER_2;

    g_const_key_hash_table_insert(clone->id_users_rel,
                                  user_get_const_id(new_user),
                                  pool_user_and_data);
    return 0;

DEFER_2:
    user_free(new_user);
DEFER_1:
    return 1;
}

user_manager_t *user_manager_clone(const user_manager_t *manager) {
    user_manager_t *const clone = user_manager_create();
    if (!clone)
        return NULL;

    if (pool_iter(manager->user_data, __user_manager_clone_foreach_user_data, clone)) {
        user_manager_free(clone);
        return NULL;
    }
    return clone;
}

int user_manager_add_user(user_manager_t *manager, const user_t *user) {
    const user_t *const pool_user = user_clone(manager->users, manager->strings, user);
    if (!pool_user)
        return 1;

    const user_manager_user_and_data_t user_and_data = {
        .user         = pool_user,
        .flights      = single_pool_id_linked_list_create(),
        .reservations = single_pool_id_linked_list_create(),
    };

    user_manager_user_and_data_t *const pool_user_and_data =
        pool_put_item(user_manager_user_and_data_t, manager->user_data, &user_and_data);
    if (!pool_user_and_data)
        return 1;

    if (!g_const_key_hash_table_insert(manager->id_users_rel,
                                       user_get_const_id(pool_user),
                                       pool_user_and_data)) {

        /* Do not fatally fail (just print a warning). Show must go on. */
        fprintf(stderr,
                "REPEATED USER ID \"%s\". This shouldn't happen! Replacing it.\n",
                user_get_const_id(user));
    }

    return 0;
}

int user_manager_add_user_flight_association(user_manager_t *manager,
                                             const char     *user_id,
                                             flight_id_t     flight_id) {

    user_manager_user_and_data_t *const data =
        g_const_key_hash_table_lookup(manager->id_users_rel, user_id);
    if (!data)
        return 1;

    single_pool_id_linked_list_t *const tmp =
        single_pool_id_linked_list_append_beginning(manager->ll_nodes, data->flights, flight_id);
    if (!tmp)
        return 1;

    data->flights = tmp;
    return 0;
}

int user_manager_add_user_reservation_association(user_manager_t  *manager,
                                                  const char      *user_id,
                                                  reservation_id_t reservation_id) {

    user_manager_user_and_data_t *const data =
        g_const_key_hash_table_lookup(manager->id_users_rel, user_id);
    if (!data)
        return 1;

    single_pool_id_linked_list_t *const tmp =
        single_pool_id_linked_list_append_beginning(manager->ll_nodes,
                                                    data->reservations,
                                                    reservation_id);
    if (!tmp)
        return 1;

    data->reservations = tmp;
    return 0;
}

const user_t *user_manager_get_by_id(const user_manager_t *manager, const char *id) {
    const user_manager_user_and_data_t *const data =
        g_const_key_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->user;
}

const single_pool_id_linked_list_t *user_manager_get_flights_by_id(const user_manager_t *manager,
                                                                   const char           *id) {

    const user_manager_user_and_data_t *const data =
        g_const_key_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->flights;
}

const single_pool_id_linked_list_t *
    user_manager_get_reservations_by_id(const user_manager_t *manager, const char *id) {

    const user_manager_user_and_data_t *const data =
        g_const_key_hash_table_lookup(manager->id_users_rel, id);
    if (!data)
        return NULL;
    return data->reservations;
}

int user_manager_iter(const user_manager_t        *manager,
                      user_manager_iter_callback_t callback,
                      void                        *user_data) {

    return pool_iter(manager->users, (pool_iter_callback_t) callback, user_data);
}

/**
 * @struct user_manager_iter_with_flights_data_t
 * @brief Auxiliary data for ::__user_manager_iter_with_flights_callback.
 *
 * @var user_manager_iter_with_flights_data_t::user_data
 *     @brief Original user data provided to ::user_manager_iter_with_flights.
 * @var user_manager_iter_with_flights_data_t::original_callback
 *     @brief Original callback provided to ::user_manager_iter_with_flights.
 */
typedef struct {
    void *const                                     user_data;
    const user_manager_iter_with_flights_callback_t original_callback;
} user_manager_iter_with_flights_data_t;

/**
 * @brief Intermediate callback for ::user_manager_iter_with_flights.
 *
 * @param iter_data Real callback and original user data (::user_manager_iter_with_flights_data_t).
 * @param user_data A pointer to a ::user_and_data_t.
 *
 * @return The value of the original callback called.
 */
int __user_manager_iter_with_flights_callback(void *iter_data, const void *user_data) {
    const user_manager_iter_with_flights_data_t *const iter_data_struct = iter_data;
    const user_manager_user_and_data_t *const          user_data_struct = user_data;

    return iter_data_struct->original_callback(iter_data_struct->user_data,
                                               user_data_struct->user,
                                               user_data_struct->flights);
}

int user_manager_iter_with_flights(const user_manager_t                     *manager,
                                   user_manager_iter_with_flights_callback_t callback,
                                   void                                     *user_data) {

    user_manager_iter_with_flights_data_t iter_data = {.user_data         = user_data,
                                                       .original_callback = callback};
    return pool_iter(manager->user_data, __user_manager_iter_with_flights_callback, &iter_data);
}

void user_manager_free(user_manager_t *manager) {
    pool_free(manager->users);
    pool_free(manager->user_data);
    pool_free(manager->ll_nodes);
    string_pool_free(manager->strings);
    g_const_key_hash_table_unref(manager->id_users_rel);
    free(manager);
}
