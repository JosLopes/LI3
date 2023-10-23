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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "types/user.h"

struct user {
    char           *id;
    char           *name;
    char           *passport;
    char           *country_code;
    enum sex        sex;
    bool            active_status;
    date_and_time_t account_creation_date;
};

user_t *user_create(void) {
    user_t *new_user       = malloc(sizeof(user_t));
    new_user->name         = NULL;
    new_user->passport     = NULL;
    new_user->country_code = NULL;

    return new_user;
}

void user_set_id(user_t *user, char *id) {
    user->id = strdup(id);
}

void user_set_name(user_t *user, char *name) {
    user->name = strdup(name);
}

void user_set_passport(user_t *user, char *passport) {
    user->passport = strdup(passport);
}

void user_set_country_code(user_t *user, char *country_code) {
    user->country_code = strdup(country_code);
}

void user_set_sex(user_t *user, char *sex_string) {
    enum sex sex;

    if (!strcmp(sex_string, "M")) {
        sex = M;
    } else {
        sex = F;
    }

    user->sex = sex;
}

void user_set_account_status(user_t *user, bool active_status) {
    user->active_status = active_status;
}

void user_set_account_creation_date(user_t *user, date_and_time_t date) {
    user->account_creation_date = date;
}

const char *user_get_const_id(user_t *user) {
    return user->id;
}

const char *user_get_const_name(user_t *user) {
    return user->name;
}

const char *user_get_const_passport(user_t *user) {
    return user->country_code;
}

const char *user_get_const_country_code(user_t *user) {
    return user->country_code;
}

enum sex user_get_sex(user_t *user) {
    return user->sex;
}

bool user_get_account_status(user_t *user) {
    return user->active_status;
}

date_and_time_t user_get_account_creation_date(user_t *user) {
    return user->account_creation_date;
}

void user_free(user_t *user) {
    free(user->id);
    free(user->name);
    free(user->passport);
    free(user->country_code);

    free(user);
}
