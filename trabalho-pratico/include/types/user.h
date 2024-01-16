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
 * @file  user.h
 * @brief Declaration of type ::user_t.
 *
 * @details See what fields define a user (and thus available through getters and setters) in the
 *          [struct's documentation](@ref user).
 *
 * @anchor user_examples
 * ### Examples
 *
 * See [the examples in user_manager.h](@ref user_manager_examples). The callback there,
 * `iter_callback` is a great example on how to extract all data from an existing user and print it
 * to `stdout`.
 */

#ifndef USER_H
#define USER_H

#include "types/account_status.h"
#include "types/country_code.h"
#include "types/sex.h"
#include "utils/date_and_time.h"
#include "utils/pool.h"
#include "utils/string_pool.h"

/** @brief A user. */
typedef struct user user_t;

/**
 * @brief   Creates a new user with uninitialized fields.
 * @details Before using this user, set all its fields using the setters in this module.
 *
 * @param allocator  Pool where to allocate the user. Its element size must be the value returned by
 *                   ::user_sizeof. Can be `NULL`, so that malloc is used instead of a pool.
 *
 * @return A allocated user (`NULL` on allocation failure).
 */
user_t *user_create(pool_t *allocator);

/**
 * @brief Creates a deep clone of a user.
 *
 * @param allocator        Pool where to allocate the user. Its element size must be the value
 *                         returned by ::user_sizeof. Can be `NULL`, so that malloc is used, instead
 *                         of a pool.
 * @param string_allocator Pool where to allocate the strings of a user. Can be `NULL`, so that
 *                         `strdup` is used, instead of a pool.
 * @param user             User to be cloned.
 *
 * @return A deep-clone of @p user (`NULL` on allocation failure).
 */
user_t *user_clone(pool_t *allocator, string_pool_t *string_allocator, const user_t *user);

/**
 * @brief Sets a user's identifier.
 *
 * @param allocator Pool where to allocate @p id. `NULL` can be provided so that `strdup` is used
 *                  instead of a pool.
 * @param user      User to have its identifier set.
 * @param id        Identifier of the user. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p id or allocation failure. @p user wasn't modified.
 */
int user_set_id(string_pool_t *allocator, user_t *user, const char *id);

/**
 * @brief Sets a user's name.
 *
 * @param allocator Pool where to allocate @p name. `NULL` can be provided so that `strdup` is used
 *                  instead of a pool.
 * @param user      User to have its name set.
 * @param name      Name of the user. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p name or allocation failure. @p user wasn't modified.
 */
int user_set_name(string_pool_t *allocator, user_t *user, const char *name);

/**
 * @brief Sets a user's birth date.
 * @param user       User to have its birth date set.
 * @param birth_date Birth date of the user. Must be before the user's account creation date. If
 *                   that date hasn't been initialized, that comparison won't be performed.
 *
 * @retval 0 Success.
 * @retval 1 Failure because @p birth_date doesn't come before `account_creation_date`. @p user
 *           wasn't modified.
 */
int user_set_birth_date(user_t *user, date_t birth_date);

/**
 * @brief Sets a user's passport.
 *
 * @param allocator Pool where to allocate @p passport. `NULL` can be provided so that `strdup` is
 *                  used instead of a pool.
 * @param user      User to have its passport set.
 * @param passport  Passport number of the user. Musn't be empty.
 *
 * @retval 0 Success.
 * @retval 1 Failure due to empty @p passport or allocation failure. @p user wasn't modified.
 */
int user_set_passport(string_pool_t *allocator, user_t *user, const char *passport);

/**
 * @brief Sets a user's country code.
 * @param user         User to have its country code set.
 * @param country_code Country code of the user.
 */
void user_set_country_code(user_t *user, country_code_t country_code);

/**
 * @brief Sets a user's sex.
 * @param user User to have its sex set.
 * @param sex  Sex of the user.
 */
void user_set_sex(user_t *user, sex_t sex);

/**
 * @brief Sets a user's account status.
 * @param user           User to have its account status set.
 * @param account_status Whether the user's account is active or inactive.
 */
void user_set_account_status(user_t *user, account_status_t account_status);

/**
 * @brief Sets a user's account creation date.
 * @param user                  User to have its account status set.
 * @param account_creation_date Account creation date of the user. Must be after the user's birth
 *                              date. If that date hasn't been initialized, that comparison won't be
 *                              performed.
 */
int user_set_account_creation_date(user_t *user, date_and_time_t account_creation_date);

/**
 * @brief   Resets the birth date and account creation date in a user.
 * @details This is done so that the setters ::user_set_birth_date and
 *          ::user_set_account_creation_date can work without previous dates making validity
 *          comparisons fail.
 *
 *          You must call both setters after calling this method.
 *
 * @param user User to have its dates reset.
 */
void user_reset_dates(user_t *user);

/**
 * @brief  Gets a user's identifier.
 * @param  user User to get the identifier from.
 * @return The user's identifier.
 */
const char *user_get_const_id(const user_t *user);

/**
 * @brief  Gets a user's name.
 * @param  user User to get the name name from.
 * @return The user's name.
 */
const char *user_get_const_name(const user_t *user);

/**
 * @brief  Gets a user's birth date.
 * @param  user User to get the birth date from.
 * @return The user's birth date.
 */
date_t user_get_birth_date(const user_t *user);

/**
 * @brief  Gets the user passport.
 * @param  user User to get the passport number from.
 * @return The user's passport number.
 */
const char *user_get_const_passport(const user_t *user);

/**
 * @brief  Gets a user's country code.
 * @param  user User to get the country code from.
 * @return The country code of @p user.
 */
country_code_t user_get_country_code(const user_t *user);

/**
 * @brief  Gets a user's sex.
 * @param  user User to get the sex from.
 * @return The user's sex.
 */
sex_t user_get_sex(const user_t *user);

/**
 * @brief  Gets a user's account_status.
 * @param  user User to get the account status from.
 * @return The user's account status.
 */
account_status_t user_get_account_status(const user_t *user);

/**
 * @brief  Gets a user's account creation date.
 * @param  user User to get the account creation date from.
 * @return The user's account creation date.
 */
date_and_time_t user_get_account_creation_date(const user_t *user);

/**
 * @brief   Gets the size of a ::user_t in memory.
 * @details Useful for pool allocation.
 * @return  `sizeof(user_t)`.
 */
size_t user_sizeof(void);

/**
 * @brief   Checks if a user in a database is valid.
 * @details Users can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param user User to have its validity checked.
 *
 * @retval 0 Valid user.
 * @retval 1 Invalid user.
 */
int user_is_valid(const user_t *user);

/**
 * @brief   Alters a user in a database to make it invalid.
 * @details Users can be invalidated so that they can be "removed" from pools (not considered
 *          during lookups), even though pools don't support removals.
 *
 * @param user User to be modified.
 */
void user_invalidate(user_t *user);

/**
 * @brief   Calculates the age of @p user.
 * @details The current date is considered to be ::DATE_CURRENT.
 *
 * @param  user User to have its age calculated.
 *
 * @return Age of @p user in years.
 */
int32_t user_calculate_age(const user_t *user);

/**
 * @brief      Frees the memory used for a given user.
 * @details    All strings inside the user won't be freed, as they're not owned by the user.
 * @param user User to be deleted.
 */
void user_free(user_t *user);

#endif
