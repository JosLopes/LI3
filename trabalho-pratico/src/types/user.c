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
 */

#include <stdlib.h>

#include "types/user.h"

/**
 * @struct user
 * @brief Represents a user in the datasets.
 * @details NOTE: some fields in the project's requirements (such as emails, phone numbers,
 *          addresses and payment methods) aren't put here, as they won't be required by any
 *          database query.
 *
 * @var user::id
 *     @brief Identifier of a given user.
 * @var user::name
 *     @brief Full name of a given user.
 * @var user::birth_date
 *     @brief Date of birth of the user.
 * @var user::sex
 *     @brief Sex of the user.
 * @var user::passport
 *     @brief Passport number of a given user.
 * @var user::country_code
 *     @brief Code of the country of a given user.
 * @var user::account_creation_date
 *     @brief Date of creation of the user's account.
 * @var user::account_status
 *     @brief Whether a user's account is active or inactive.
 */
struct user {
    char            *id;
    char            *name;
    date_t           birth_date;
    char            *passport;
    char            *country_code;
    sex_t            sex;
    account_status_t account_status;
    date_and_time_t  account_creation_date;
};

user_t *user_create(void) {
    return malloc(sizeof(struct user));
}

void user_set_id(user_t *user, char *id) {
    user->id = id;
}

void user_set_name(user_t *user, char *name) {
    user->name = name;
}

void user_set_birth_date(user_t *user, date_t date) {
    user->birth_date = date;
}

void user_set_passport(user_t *user, char *passport) {
    user->passport = passport;
}

void user_set_country_code(user_t *user, char *country_code) {
    user->country_code = country_code;
}

void user_set_sex(user_t *user, sex_t sex) {
    user->sex = sex;
}

void user_set_account_status(user_t *user, account_status_t account_status) {
    user->account_status = account_status;
}

void user_set_account_creation_date(user_t *user, date_and_time_t date) {
    user->account_creation_date = date;
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
    return user->country_code;
}

const char *user_get_const_country_code(const user_t *user) {
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

void user_free(user_t *user) {
    free(user);
}
