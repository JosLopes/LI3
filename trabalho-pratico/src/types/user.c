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

#include "types/user.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct user {
    char    *id;
    char    *name;
    char    *passport;
    char    *country_code;
    enum sex sex;
    bool     active_status;
    int      account_creation_date;
};

user_t *create_user(void) {
    user_t *new_user       = malloc(sizeof(user_t));
    new_user->name         = NULL;
    new_user->passport     = NULL;
    new_user->country_code = NULL;

    return new_user;
}

void set_user_id(user_t *user, char *id) {
    user->id = strdup(id);
}

void set_user_name(user_t *user, char *name) {
    user->name = strdup(name);
}

void set_user_passport(user_t *user, char *passport) {
    user->passport = strdup(passport);
}

void set_user_country_code(user_t *user, char *country_code) {
    user->country_code = strdup(country_code);
}

void set_user_sex(user_t *user, char *sex_string) {
    enum sex sex;

    if (!strcmp(sex_string, "M")) {
        sex = M;
    } else {
        sex = F;
    }

    user->sex = sex;
}

void set_user_account_status(user_t *user, bool active_status) {
    user->active_status = active_status;
}

void set_user_account_creation_date(user_t *user, int date) {
    user->account_creation_date = date;
}

const char *get_const_user_id(user_t *user) {
    return user->id;
}

const char *get_const_user_name(user_t *user) {
    return user->name;
}

const char *get_const_user_passport(user_t *user) {
    return user->country_code;
}

const char *get_const_user_country_code(user_t *user) {
    return user->country_code;
}

enum sex get_user_sex(user_t *user) {
    return user->sex;
}

bool get_user_account_status(user_t *user) {
    return user->active_status;
}

int get_user_account_creation_date(user_t *user) {
    return user->account_creation_date;
}

void free_user(user_t *user) {
    free(user->name);
    free(user->passport);
    free(user->country_code);

    free(user);
}
