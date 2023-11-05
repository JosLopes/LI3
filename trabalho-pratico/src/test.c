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
 * @file test.c
 * @brief Contains the entry point to the program.
 */

#include <stdio.h>

#include "database/user_manager.h"
#include "dataset/dataset_loader.h"
#include "types/user.h"
#include "utils/pool.h"

/**
 * @brief Callback called for every user in the database, that prints it to the screen.
 *
 * @param user_data `NULL`.
 * @param user      User to be printed to `stdout`.
 *
 * @retval Always `0`, as this cannot fail.
 */
int iter_callback(void *user_data, user_t *user) {
    (void) user_data;

    const char *id       = user_get_const_id(user);
    const char *name     = user_get_const_name(user);
    const char *passport = user_get_const_passport(user);

    char country_code[COUNTRY_CODE_SPRINTF_MIN_BUFFER_SIZE];
    country_code_sprintf(country_code, user_get_country_code(user));

    char birth_date[DATE_SPRINTF_MIN_BUFFER_SIZE];
    date_sprintf(birth_date, user_get_birth_date(user));

    const char *account_status =
        user_get_account_status(user) == ACCOUNT_STATUS_INACTIVE ? "inactive" : "active";

    char account_creation_date[DATE_AND_TIME_SPRINTF_MIN_BUFFER_SIZE];
    date_and_time_sprintf(account_creation_date, user_get_account_creation_date(user));

    printf("--- USER ---\nid: %s\nname: %s\npassport: %s\ncountry_code: %s\nbirth_date: "
           "%s\naccount_status: %s\naccount_creation_date: %s\n\n",
           id,
           name,
           passport,
           country_code,
           birth_date,
           account_status,
           account_creation_date);
    return 0;
}

/**
 * @brief The entry point to the test program.
 * @details Tests for dataset parsing.

 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    database_t *database = database_create();
    if (!database) {
        fprintf(stderr, "Failed to allocate database!");
        return 1;
    }

    if (dataset_loader_load(database, "/home/voidbert/Uni/3/LI3/dataset/data")) {
        fputs("Failed to open dataset to be parsed.\n", stderr);
        return 1;
    }

    user_manager_iter(database_get_users(database), iter_callback, NULL);

    database_free(database);
    return 0;
}
