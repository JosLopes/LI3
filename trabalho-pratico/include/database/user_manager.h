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
 * @file    user_manager.h
 * @brief   Contains and manages all users in a database.
 */

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "types/user.h"

/**
 * @brief A data type that contains and manages all users in a database.
 */
typedef struct user_manager user_manager_t;

/**
 * @brief   Callback type for user manager iterations.
 * @details Method called by ::user_manager_iter for every item in a ::user_manager_t.
 *
 * @param user_data Argument passed to ::user_manager_iter that is passed to every callback, so
 *                  that this method can change the program's state.
 * @param user      User in the manager.
 *
 * @return `0` on success, or any other value to order iteration to stop.
 */
typedef int (*user_manager_iter_callback_t)(void *user_data, user_t *user);

/**
 * @brief   Instantiates a new ::user_manager_t.
 * @details The returned value is owned by the called and should be `free`'d with
 *          ::user_manager_free.
 * @return  The new user manager, or `NULL` on failure.
 */
user_manager_t *user_manager_create(void);

/**
 * @brief Adds a user to the user manager.
 *
 * @param manager User manager to add @p user to.
 * @param user    User to be added to @p manager.
 *
 * @return The pointer to the user allocated in the manager's pool, or `NULL` on failure.
 */
user_t *user_manager_add_user(user_manager_t *manager, const user_t *user);

/**
 * @brief Gets a user stored in @p manager by its identifier.
 *
 * @param manager User manager where to perform the lookup.
 * @param id      Identifier of the user to find.
 *
 * @return A ::user_t if it's found, `NULL` if it's not.
 */
user_t *user_manager_get_by_id(const user_manager_t *manager, const char *id);

/**
 * @brief Iterates through every **valid** user in a user manager, calling @p callback for each one.
 *
 * @param manager   User manager to iterate thorugh.
 * @param callback  Method to be called for every user stored in @p manager.
 * @param user_data Pointer to be passed to every @p callback, so that it can modify the program's
 *                  state.
 *
 * @return The return value of the last-called @p callback.
 */
int user_manager_iter(user_manager_t              *manager,
                      user_manager_iter_callback_t callback,
                      void                        *user_data);

/**
 * @brief Frees memory used by a user manager.
 * @param manager User manager whose memory is to be `free`'d.
 */
void user_manager_free(user_manager_t *manager);

#endif
