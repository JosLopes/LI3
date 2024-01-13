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
 * @file  user.c
 * @brief Implementation of methods in include/types/user.h
 *
 * ### Example
 * See [the header file's documentation](@ref user_examples).
 */

#include <stdlib.h>
#include <string.h>

#include "types/user.h"

/**
 * @struct  user
 * @brief   A user.
 * @details Some fields in the project's requirements (such as emails, phone numbers, addresses
 *          and payment methods) aren't put here, as they aren't required by any of the queries.
 *
 * @var user::id
 *     @brief Identifier of a given user.
 * @var user::name
 *     @brief Full name of a given user.
 * @var user::birth_date
 *     @brief Date of birth of a given user.
 * @var user::sex
 *     @brief Sex of a given user.
 * @var user::passport
 *     @brief Passport number of a given user.
 * @var user::country_code
 *     @brief Code of the country of a given user.
 * @var user::account_creation_date
 *     @brief Date of creation of a given user's account.
 * @var user::account_status
 *     @brief Whether a user's account is active or inactive.
 * @var user::owns_itself
 *     @brief   Whether, when `free`ing this user, the user pointer should be `free`'d.
 *     @details A false value means that the user is allocated in a pool.
 * @var user::owns_id
 *     @brief   Whether ::user::id should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 * @var user::owns_name
 *     @brief   Whether ::user::name should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 * @var user::owns_passport
 *     @brief   Whether ::user::passport should be `free`d.
 *     @details A false value means that this string is allocated in a pool.
 */
struct user {
    char            *id;
    char            *name;
    char            *passport;
    date_and_time_t  account_creation_date;
    date_t           birth_date;
    country_code_t   country_code;
    sex_t            sex            : 1;
    account_status_t account_status : 1;

    int owns_itself : 1, owns_id : 1, owns_name : 1, owns_passport : 1;
};

user_t *user_create(pool_t *allocator) {
    user_t *ret = allocator ? pool_alloc_item(user_t, allocator) : malloc(sizeof(user_t));
    if (!ret)
        return NULL;

    ret->owns_itself = allocator == NULL;
    ret->owns_id = ret->owns_name = ret->owns_passport = 0; /* Don't free in first setter call */
    user_reset_dates(ret); /* For first comparisons to work */
    return ret;
}

user_t *user_clone(pool_t *allocator, string_pool_t *string_allocator, const user_t *user) {
    user_t *ret = user_create(allocator);
    if (!ret)
        return NULL;

    memcpy(ret, user, sizeof(user_t));
    ret->owns_itself = allocator == NULL;
    ret->owns_id = ret->owns_name = ret->owns_passport = 0; /* Don't free in first setter call */

    if (user_set_id(string_allocator, ret, user->id) ||
        user_set_name(string_allocator, ret, user->name) ||
        user_set_passport(string_allocator, ret, user->passport)) {

        if (ret->owns_itself)
            free(ret);
        return NULL;
    }

    return ret;
}

int user_set_id(string_pool_t *allocator, user_t *user, const char *id) {
    if (!*id)
        return 1;

    char *new_id = allocator ? string_pool_put(allocator, id) : strdup(id);
    if (!new_id)
        return 1;

    if (user->owns_id)
        free(user->id);
    user->owns_id = allocator == NULL;

    user->id = new_id;
    return 0;
}

int user_set_name(string_pool_t *allocator, user_t *user, const char *name) {
    if (!*name)
        return 1;

    char *new_name = allocator ? string_pool_put(allocator, name) : strdup(name);
    if (!new_name)
        return 1;

    if (user->owns_name)
        free(user->name);
    user->owns_name = allocator == NULL;

    user->name = new_name;
    return 0;
}

int user_set_birth_date(user_t *user, date_t birth_date) {
    if (date_diff(birth_date, date_and_time_get_date(user->account_creation_date)) > 0)
        return 1;

    user->birth_date = birth_date;
    return 0;
}

int user_set_passport(string_pool_t *allocator, user_t *user, const char *passport) {
    if (!*passport)
        return 1;

    char *new_passport = allocator ? string_pool_put(allocator, passport) : strdup(passport);
    if (!new_passport)
        return 1;

    if (user->owns_passport)
        free(user->passport);
    user->owns_passport = allocator == NULL;

    user->passport = new_passport;
    return 0;
}

void user_set_country_code(user_t *user, country_code_t country_code) {
    user->country_code = country_code;
}

void user_set_sex(user_t *user, sex_t sex) {
    user->sex = sex;
}

void user_set_account_status(user_t *user, account_status_t account_status) {
    user->account_status = account_status;
}

int user_set_account_creation_date(user_t *user, date_and_time_t account_creation_date) {
    if (date_diff(user->birth_date, date_and_time_get_date(account_creation_date)) > 0)
        return 1;

    user->account_creation_date = account_creation_date;
    return 0;
}

void user_reset_dates(user_t *user) {
    user->birth_date            = 0;
    user->account_creation_date = 0xFFFFFFFFFFFFFFFF;
}

const char *user_get_const_id(const user_t *user) {
    return user->id;
}

const char *user_get_const_name(const user_t *user) {
    return user->name;
}

date_t user_get_birth_date(const user_t *user) {
    return user->birth_date;
}

const char *user_get_const_passport(const user_t *user) {
    return user->passport;
}

country_code_t user_get_country_code(const user_t *user) {
    return user->country_code;
}

sex_t user_get_sex(const user_t *user) {
    return user->sex;
}

account_status_t user_get_account_status(const user_t *user) {
    return user->account_status;
}

date_and_time_t user_get_account_creation_date(const user_t *user) {
    return user->account_creation_date;
}

size_t user_sizeof(void) {
    return sizeof(user_t);
}

int user_is_valid(const user_t *user) {
    return user->id == NULL;
}

void user_invalidate(user_t *user) {
    if (user->owns_id)
        free(user->id);
    user->id = NULL;
}

int32_t user_calculate_age(const user_t *user) {
    return date_diff(DATE_CURRENT, user->birth_date) / 372;
}

void user_free(user_t *user) {
    if (user->owns_id)
        free(user->id);
    if (user->owns_name)
        free(user->name);
    if (user->owns_passport)
        free(user->passport);

    if (user->owns_itself)
        free(user);
}
