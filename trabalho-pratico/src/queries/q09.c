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
 * @file  q09.c
 * @brief Implementation of methods in include/queries/q09.h
 */

#include <ctype.h>
#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queries/q09.h"
#include "queries/query_instance.h"

/**
 * @brief   Parses arguments of a query of type 9.
 * @details Asserts there's only one string argument, that is stored.
 *
 * @param argv Values of the arguments.
 * @param argc Number of arguments.
 *
 * @return `NULL` for invalid arguments, a copy of the only @p argv on success.
 */
void *__q09_parse_arguments(char **argv, size_t argc) {
    if (argc != 1)
        return NULL;
    else
        return strdup(argv[0]);
}

/**
 * @struct q09_execute_callback_user_data_t
 * @brief  Data needed while iterating through a list of users.
 *
 * @var q09_execute_callback_user_data_t::matches
 *     @brief All users found whose identifier starts with
 *            ::q09_execute_callback_user_data_t::needle.
 * @var q09_execute_callback_user_data_t::needle
 *     @brief Prefix to look for in the beginning of a user's identifier.
 * @var q09_execute_callback_user_data_t::needle_length
 *     @brief Pre-calculated `strlen(needle)`, for better performance.
 */
typedef struct {
    GPtrArray  *matches;
    const char *needle;
    size_t      needle_length;
} q09_execute_callback_user_data_t;

/**
 * @brief   Callback called for every user, to match user ID prefixes.
 * @details Auxiliary method for ::__q09_execute.
 *
 * @param user_data A pointer to a ::q09_execute_callback_user_data_t.
 * @param user      User to be inspected.
 *
 * @retval 0 Always successful.
 */
int __q09_execute_iter_callback(void *user_data, const user_t *user) {
    q09_execute_callback_user_data_t *execute_data = (q09_execute_callback_user_data_t *) user_data;

    if (strncmp(execute_data->needle, user_get_const_name(user), execute_data->needle_length) ==
            0 &&
        user_get_account_status(user) == ACCOUNT_STATUS_ACTIVE) {

        /* TODO - find a way of not keeping the const */
        g_ptr_array_add(execute_data->matches, (user_t *) user);
    }

    return 0;
}

/** @brief User comparison function for user ordering for final output. */
gint __q09_sort_compare_callback(gconstpointer a, gconstpointer b) {
    const user_t *user_a = *(const user_t **) a;
    const user_t *user_b = *(const user_t **) b;

    gint crit1 = strcoll(user_get_const_name(user_a), user_get_const_name(user_b));
    if (crit1)
        return crit1;

    gint crit2 = strcoll(user_get_const_id(user_a), user_get_const_id(user_b));
    return crit2;
}

/**
 * @brief   Executes a query of type 9.
 * @details Filthy solution due to time constraints. TODO - don't iterate over all users
 *
 * @param database   Database where to get users from
 * @param statistics `NULL`, as this query does not generate statistical data.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 Always succcessful.
 */
int __q09_execute(database_t       *database,
                  void             *statistics, /* NULL */
                  query_instance_t *instance,
                  query_writer_t   *output) {
    (void) statistics;

    char *old_locale = strdup(setlocale(LC_COLLATE, NULL));
    setlocale(LC_COLLATE, "en_US.UTF-8");

    GPtrArray                       *matches   = g_ptr_array_new();
    q09_execute_callback_user_data_t user_data = {
        .matches = matches,
        .needle  = (const char *) query_instance_get_argument_data(instance)};
    user_data.needle_length = strlen(user_data.needle);

    user_manager_iter(database_get_users(database), __q09_execute_iter_callback, &user_data);
    g_ptr_array_sort(matches, __q09_sort_compare_callback);

    for (size_t i = 0; i < matches->len; ++i) {
        user_t *user = (user_t *) g_ptr_array_index(matches, i);
        query_writer_write_new_object(output);
        query_writer_write_new_field(output, "id", "%s", user_get_const_id(user));
        query_writer_write_new_field(output, "name", "%s", user_get_const_name(user));
    }

    g_ptr_array_free(matches, TRUE);

    if (old_locale) {
        setlocale(LC_COLLATE, old_locale);
        free(old_locale);
    }

    return 0;
}

query_type_t *q09_create(void) {
    return query_type_create(__q09_parse_arguments, free, NULL, NULL, __q09_execute);
}
