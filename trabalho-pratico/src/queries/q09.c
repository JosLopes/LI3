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
void *__q09_parse_arguments(size_t argc, char *const argv[argc]) {
    if (argc != 1)
        return NULL;
    else
        return strdup(argv[0]);
}

/**
 * @struct q09_statistical_data_t
 * @brief Query 9's statistical data.
 *
 * @var q09_statistical_data_t::prefixes
 *     @brief Sorted array of prefixes to compare user names against.
 * @var q09_statistical_data_t::lengths
 *     @brief Length of each prefix in ::q09_statistical_data_t::prefixes.
 * @var q09_statistical_data_t::matches
 *     @brief Arrays of pointers to users that match with each prefix in
 *            q09_statistical_data_t::prefixes.
 * @var q09_statistical_data_t::n
 *     @brief Number of elements in every array of this struct.
 */
typedef struct {
    const char **prefixes;
    size_t      *lengths;
    GPtrArray  **matches;
    size_t       n;
} q09_statistical_data_t;

/**
 * @brief   Callback called for every user, to match user ID prefixes.
 * @details Auxiliary method for ::__q09_generate_statistics.
 *
 * @param user_data A pointer to a ::q09_statistical_data_t.
 * @param user      User to be inspected.
 *
 * @retval 0 Always successful.
 */
int __q09_generate_statistics_iter_callback(void *user_data, const user_t *user) {
    q09_statistical_data_t *stats = (q09_statistical_data_t *) user_data;

    /* Local copies for slight performance increase */
    size_t       n        = stats->n;
    const char **prefixes = stats->prefixes;
    size_t      *lengths  = stats->lengths;

    for (size_t i = 0; i < n; ++i) {
        int cmp = strncmp(prefixes[i], user_get_const_name(user), lengths[i]);

        if (cmp == 0 && user_get_account_status(user) == ACCOUNT_STATUS_ACTIVE) {
            /* TODO - find a way of not keeping the const */
            g_ptr_array_add(stats->matches[i], (user_t *) user);
        } else if (cmp > 0) {
            break;
        }
    }

    return 0;
}

/** @brief User comparison function for user ordering for final output. */
gint __q09_sort_compare_callback(gconstpointer a, gconstpointer b) {
    const user_t *user_a = *(const user_t *const *) a;
    const user_t *user_b = *(const user_t *const *) b;

    gint crit1 = strcoll(user_get_const_name(user_a), user_get_const_name(user_b));
    if (crit1)
        return crit1;

    gint crit2 = strcoll(user_get_const_id(user_a), user_get_const_id(user_b));
    return crit2;
}

/**
 * @brief   Comparison function for sorting arrays of strings.
 * @details Used in ::__q09_generate_statistics to sort ::q09_statistical_data_t::prefixes.
 */
int __q09_sort_prefixes_callback(const void *a, const void *b) {
    const char *ap = *(const char *const *) a;
    const char *bp = *(const char *const *) b;
    return strcmp(ap, bp);
}

/**
 * @brief Generates statistical data for queries of type 9.
 *
 * @param database  Database, to iterate through users.
 * @param n         Number of query instances that will need to be executed.
 * @param instances Query instances that will need to be executed.
 *
 * @return A pointer to a ::q09_statistical_data_t.
 */
void *__q09_generate_statistics(const database_t             *database,
                                size_t                        n,
                                const query_instance_t *const instances[n]) {

    /* Set locale for sorting and restore older locale later */
    char *old_locale = strdup(setlocale(LC_COLLATE, NULL));
    setlocale(LC_COLLATE, "en_US.UTF-8");

    /* Allocate and initialize statistics */
    q09_statistical_data_t *stats = malloc(sizeof(q09_statistical_data_t));
    if (!stats)
        return NULL;

    stats->prefixes = malloc(sizeof(const char *) * n);
    stats->lengths  = malloc(sizeof(size_t) * n);
    stats->matches  = malloc(sizeof(GPtrArray **) * n);
    stats->n        = n;

    if (!stats->prefixes || !stats->lengths || !stats->matches) {
        if (stats->prefixes)
            free(stats->prefixes);
        if (stats->lengths)
            free(stats->lengths);
        if (stats->matches)
            free(stats->matches);

        free(stats);
        return NULL;
    }

    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        const char *prefix = query_instance_get_argument_data(instances[i]);

        int found = 0;
        for (size_t j = 0; j < count; ++j) {
            if (strcmp(prefix, stats->prefixes[j]) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            stats->prefixes[count] = prefix;
            count++;
        }
    }
    stats->n = count;

    qsort(stats->prefixes, count, sizeof(const char *), __q09_sort_prefixes_callback);

    for (size_t i = 0; i < stats->n; ++i) {
        stats->lengths[i] = strlen(stats->prefixes[i]);
        stats->matches[i] = g_ptr_array_new();
    }

    /* Fill matches in statistical data */
    user_manager_iter(database_get_users(database), __q09_generate_statistics_iter_callback, stats);

    for (size_t i = 0; i < stats->n; ++i) {
        g_ptr_array_sort(stats->matches[i], __q09_sort_compare_callback);
    }

    if (old_locale) {
        setlocale(LC_COLLATE, old_locale);
        free(old_locale);
    }
    return stats;
}

/**
 * @brief Frees statistical data generated by ::__q09_generate_statistics.
 * @param statistical_data Value returned by ::__q09_generate_statistics.
 */
void __q09_free_statistics(void *statistical_data) {
    q09_statistical_data_t *stats = (q09_statistical_data_t *) statistical_data;
    free(stats->prefixes);
    free(stats->lengths);

    for (size_t i = 0; i < stats->n; ++i)
        g_ptr_array_unref(stats->matches[i]);
    free(stats->matches);

    free(stats);
}

/**
 * @brief Executes a query of type 9.
 *
 * @param database   Ignored database (all data is gathered in ::__q09_generate_statistics).
 * @param statistics A pointer to a ::q09_statistical_data_t.
 * @param instance   Query instance to be executed.
 * @param output     Where to write the query's output to.
 *
 * @retval 0 Always succcessful.
 * @retval 1 Bad statistical data (should not happen, please raise an issue if it does).
 */
int __q09_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;

    const char             *prefix = query_instance_get_argument_data(instance);
    q09_statistical_data_t *stats  = (q09_statistical_data_t *) statistics;

    for (size_t i = 0; i < stats->n; ++i) {
        if (strcmp(stats->prefixes[i], prefix) == 0) {

            GPtrArray *matches = stats->matches[i];
            for (size_t j = 0; j < matches->len; ++j) {
                const user_t *user = g_ptr_array_index(matches, j);
                query_writer_write_new_object(output);
                query_writer_write_new_field(output, "id", "%s", user_get_const_id(user));
                query_writer_write_new_field(output, "name", "%s", user_get_const_name(user));
            }
            return 0;
        }
    }

    fprintf(stderr, "Bad statistical data in query 9! This should not happen!\n");
    return 1;
}

query_type_t *q09_create(void) {
    return query_type_create(9,
                             __q09_parse_arguments,
                             (query_type_clone_arguments_callback_t) strdup,
                             free,
                             __q09_generate_statistics,
                             __q09_free_statistics,
                             __q09_execute);
}
