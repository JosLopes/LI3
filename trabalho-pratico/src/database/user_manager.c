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
#include <stdio.h> /* For panic debug purporses */
#include <stdlib.h>

#include "database/user_manager.h"
#include "utils/pool.h"
#include "utils/string_pool.h"

/**
 * @struct user_manager
 * @brief  A data type that contains and manages all users in a database.
 *
 * @var user_manager::users
 *     @brief Set of users in the manager.
 * @var user_manager::strings
 *     @brief Pool for any string that may need to be stored in a user.
 * @var user_manager::id_users_rel
 *     @brief Hash table for identifier -> users mapping.
 */
struct user_manager {
    pool_t        *users;
    string_pool_t *strings;
    GHashTable    *id_users_rel;
};

/** @brief Number of users in each block of ::user_manager::users. */
#define USER_MANAGER_USERS_POOL_BLOCK_CAPACITY 20000

/** @brief Number of characters in each block of ::user_manager::strings. */
#define USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY 100000

user_manager_t *user_manager_create(void) {
    user_manager_t *manager = malloc(sizeof(struct user_manager));
    if (!manager)
        return NULL;

    manager->users = __pool_create(user_sizeof(), USER_MANAGER_USERS_POOL_BLOCK_CAPACITY);
    if (!manager->users) {
        free(manager);
        return NULL;
    }

    manager->strings = string_pool_create(USER_MANAGER_STRINGS_POOL_BLOCK_CAPACITY);
    if (!manager->strings) {
        pool_free(manager->users);
        free(manager);
        return NULL;
    }

    manager->id_users_rel = g_hash_table_new(g_str_hash, g_str_equal);

    return manager;
}

user_t *user_manager_add_user(user_manager_t *manager, const user_t *user) {
    user_t *pool_user = pool_put_item(user_t, manager->users, user);
    if (!pool_user)
        return NULL;

    /* Copy strings to string pool */
    char *pool_id       = string_pool_put(manager->strings, user_get_const_id(user));
    char *pool_name     = string_pool_put(manager->strings, user_get_const_name(user));
    char *pool_passport = string_pool_put(manager->strings, user_get_const_passport(user));

    if (pool_id && pool_name && pool_passport) {
        user_set_id(pool_user, pool_id);
        user_set_name(pool_user, pool_name);
        user_set_passport(pool_user, pool_passport);

        if (!g_hash_table_insert(manager->id_users_rel, pool_id, pool_user)) {
            fprintf(stderr,
                    "REPEATED USER ID \"%s\". This shouldn't happen! Replacing it.\n",
                    pool_id);
            /* Do not fail and return NULL. Show must go on */
        }

        return pool_user;
    } else {
        /* On allocation failure, it's impossible to remove anything already in a pool */
        user_invalidate(pool_user);
        return NULL;
    }
}

user_t *user_manager_get_by_id(const user_manager_t *manager, const char *id) {
    return g_hash_table_lookup(manager->id_users_rel, id);
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
    user_manager_iter_callback_t callback;
    void                        *original_user_data;
} user_manager_iter_user_data_t;

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
int __user_manager_iter_callback(void *user_data, void *item) {
    if (user_is_valid((user_t *) item) == 0) {
        user_manager_iter_user_data_t *helper_data = (user_manager_iter_user_data_t *) user_data;
        return helper_data->callback(helper_data->original_user_data, (user_t *) item);
    }

    return 0;
}

int user_manager_iter(user_manager_t              *manager,
                      user_manager_iter_callback_t callback,
                      void                        *user_data) {

    user_manager_iter_user_data_t helper_data = {.callback           = callback,
                                                 .original_user_data = user_data};
    return pool_iter(manager->users, __user_manager_iter_callback, &helper_data);
}

void user_manager_free(user_manager_t *manager) {
    pool_free(manager->users);
    string_pool_free(manager->strings);
    g_hash_table_destroy(manager->id_users_rel);
    free(manager);
}
