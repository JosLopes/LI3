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
 * @file     user.h
 * @brief    Declaration of type ::user_t.
 *
 * @details  In this module you can find a declaration of the type `user_t`, as well as getter and
 *           setter functions, which allow the developer to access any previously created user, and
 *           functions to create new users.
 *
 *           You can see what fields define a user (and thus available through getters and setters)
 *           in the [struct's documentation](@ref user).
 */

#ifndef USER_H
#define USER_H

#include "types/account_status.h"
#include "types/sex.h"
#include "utils/date.h"
#include "utils/date_and_time.h"

/**
 * @brief   Type `user_t` defined as a struct user, that stores valuable information of a given
 *          person.
 * @details It's an opaque type.
 */
typedef struct user user_t;

/**
 * @brief Creates a new user with unitialized fields.
 * @return A `malloc`-allocated user (`NULL` on allocation failure).
 */
user_t *user_create(void);

/**
 * @brief Sets the user's identifier.
 * @details @p id will not get owned by @p user, and you should free it later.
 *
 * @param user User to have its identifier set.
 * @param id   Id of the user.
 */
void user_set_id(user_t *user, char *id);

/**
 * @brief Sets the user's name.
 * @details @p name will not get owned by @p user, and you should free it later.
 *
 * @param user User to have its name set.
 * @param name Name of the user.
 */
void user_set_name(user_t *user, char *name);

/**
 * @brief Sets the user's birth date.
 *
 * @param user User to have its birth date set.
 * @param date Birth date of the user.
 */
void user_set_birth_date(user_t *user, date_t date);

/**
 * @brief Sets the user's passport.
 * @details @p passport will not get owned by @p user, and you should free it later.
 *
 * @param user     User to have its passport set.
 * @param passport Passport number of the user.
 */
void user_set_passport(user_t *user, char *passport);

/**
 * @brief Sets the user's country code.
 * @details @p country_code will not get owned by @p user, and you should free it later.
 *
 * @param user         User to have its country code set.
 * @param country_code Country code of the user.
 */
void user_set_country_code(user_t *user, char *country_code);

/**
 * @brief Sets the user's sex.
 *
 * @param user User to have its sex set.
 * @param sex  Sex of the user.
 */
void user_set_sex(user_t *user, sex_t sex);

/**
 * @brief Sets the user's account status.
 *
 * @param user           User to have its account status set.
 * @param account_status Whether the user's account is active or inactive.
 */
void user_set_account_status(user_t *user, account_status_t account_status);

/**
 * @brief Sets the user's account creation date.
 *
 * @param user User to have its account status set.
 * @param date Account creation date of the user.
 */
void user_set_account_creation_date(user_t *user, date_and_time_t date);

/**
 * @brief Gets the user's identifier.
 * @param user User to get id from.
 * @return The user's identifier, with modifications not allowed.
 */
const char *user_get_const_id(const user_t *user);

/**
 * @brief Gets the user name.
 * @param user User to get name from.
 * @return The user's name, with modifications not allowed.
 */
const char *user_get_const_name(const user_t *user);

/**
 * @brief Gets the user's birth date.
 * @param user User to get birth date from.
 * @return The user's birth date.
 */
date_t user_get_birth_date(const user_t *user);

/**
 * @brief Gets the user passport.
 * @param user User to get passport number from.
 * @return The user's passport number, with modifications not allowed.
 */
const char *user_get_const_passport(const user_t *user);

/**
 * @brief Gets the user's country code.
 * @param user User to get country code from.
 * @return The user's country code, with modifications not allowed.
 */
const char *user_get_const_country_code(const user_t *user);

/**
 * @brief Gets the user's sex.
 * @param user User to get sex from.
 * @return The user's sex.
 */
sex_t user_get_sex(const user_t *user);

/**
 * @brief Gets the user's account_status.
 * @param user User to get account status from.
 * @return The user's account status.
 */
account_status_t user_get_account_status(const user_t *user);

/**
 * @brief Gets the user's account creation date.
 * @param user User to get account create date from.
 * @return The user's account creation date.
 */
date_and_time_t user_get_account_creation_date(const user_t *user);

/**
 * @brief      Frees the memory used for a given user.
 * @details    All strings inside the user won't be freed, as they're not owned by the user.
 * @param user User to be deleted.
 */
void user_free(user_t *user);

#endif
