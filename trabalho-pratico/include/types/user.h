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
 * @brief    Declaration of type `user_t`.
 *
 * @details  In this module you can find a declaration of the type `user_t` as a struct user,
 *           as well as get and set functions, which allow's the developer access to any
 *           previously created user, or gives them the ability to create a new user.
 *           User parameters available for the developer:
 *
 * @param id                    Id of a given user, of type `char*`.
 * @param name                  Name of a given user, of type `char*`.
 * @param passport              Passport of a given user, of type `char*`.
 * @param country_code          Code of the country of a given user, of type `char*`.
 * @param sex                   Defines the sex of a given user, of type `enum sex`.
 * @param active_status         Defines the status of an account (active/inactive), of type `bool`.
 * @param account_creation_date Defines the date of account creation, of type `date_and_time_t`.
 */

#ifndef USER_H
#define USER_H

#include <stdbool.h>

#include "utils/date_and_time.h"

/**
 * @brief Enum sex, F for female, M for male.
 */
enum sex {
    F,
    M
};

/**
 * @brief Type `user_t` defined as a struct user,
 *        stores valuable information of a given person (Opaque type).
 */
typedef struct user user_t;

/**
 * @brief Creates a new user.
 *
 * @return new_user, the new user created of type `user_t*`.
 */
user_t *user_create(void);

/**
 * @brief Sets the user's id, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_user).
 *
 * @param user User of type `user_t*`.
 * @param id   Id of a given user, of type `char*`.
 */
void user_set_id(user_t *user, char *id);

/**
 * @brief Sets the user's name, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_user).
 *
 * @param user User of type `user_t*`.
 * @param name Name of a given user, of type `char*`.
 */
void user_set_name(user_t *user, char *name);

/**
 * @brief Sets the user's passport, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_user).
 *
 * @param user     User of type `user_t*`.
 * @param passport Passport of a given user, of type `char*`.
 */
void user_set_passport(user_t *user, char *passport);

/**
 * @brief Sets the user's country code, allocating the necessary space.
 *        Be sure to free it when no longer necessary (using the function free_user).
 *
 * @param user         User of type `user_t*`.
 * @param country_code Country code of a given user, of type `char*`.
 */
void user_set_country_code(user_t *user, char *country_code);

/**
 * @brief Sets the user sex (Male of Female).
 *
 * @param user       User of type `user_t*`.
 * @param sex_string Sex of a given user, of type `char*`.
 */
void user_set_sex(user_t *user, char *sex_string);

/**
 * @brief Sets the user account status.
 *
 * @param user          User of type `user_t*`.
 * @param active_status True if account_status is active, False if it isn't, of type `bool`.
 */
void user_set_account_status(user_t *user, bool active_status);

/**
 * @brief Sets the user account creation date.
 *
 * @param user User of type `user_t*`.
 * @param date Account creation date of a given user, of type `date_and_time_t`.
 */
void user_set_account_creation_date(user_t *user, date_and_time_t date);

/**
 * @brief Gets the user id.
 * @param user User of type `user_t*`.
 * @return The user's id, of type `const char*`.
 */
const char *user_const_get_id(user_t *user);

/**
 * @brief Gets the user name.
 * @param user User of type `user_t*`.
 * @return The user's name, of type `const char*`.
 */
const char *user_const_get_name(user_t *user);

/**
 * @brief Gets the user passport.
 * @param user User of type `user_t*`.
 * @return The user's passport, of type `const char*`.
 */
const char *user_const_get_passport(user_t *user);

/**
 * @brief Gets the user country code.
 * @param user User of type `user_t*`.
 * @return The user's country_code, of type `const char*`.
 */
const char *user_const_get_country_code(user_t *user);

/**
 * @brief Gets the user sex.
 * @param user User of type `user_t*`.
 * @return The user's sex, of type `enum sex`.
 */
enum sex user_get_sex(user_t *user);

/**
 * @brief Gets the user account_status, True if the account is active.
 * @param user User of type `user_t*`.
 * @return The user's account_status, of type `bool`.
 */
bool user_get_account_status(user_t *user);

/**
 * @brief Gets the user account creation date.
 * @param user User of type `user_t*`.
 * @return The user's account_creation_date, of type int.
 */
date_and_time_t user_get_account_creation_date(user_t *user);

/**
 * @brief Function that frees the memory used for a given user, effectively deleting the user.
 * @param user User of type `user_t*`.
 */
void user_free(user_t *user);

#endif
