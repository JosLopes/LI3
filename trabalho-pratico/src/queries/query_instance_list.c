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
 * @file  query_instance_list.c
 * @brief Implementation of methods in include/queries/query_instance_list.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_instance_list_examples).
 */

#include <glib.h>
#include <stdint.h>

#include "queries/query_instance_list.h"

/**
 * @struct query_instance_list
 * @brief  A container for a sorted list of ::query_instance_t.
 *
 * @var query_instance_list::list
 *     @brief Actual sorted list of ::query_instance_t.
 * @var query_instance_list::sorted
 *     @brief If the list is sorted. When performing an iteration, the array will be sorted so that
 *            this becomes `1`.
 */
struct query_instance_list {
    GPtrArray *list;
    int        sorted;
};

query_instance_list_t *query_instance_list_create(void) {
    query_instance_list_t *list = malloc(sizeof(struct query_instance_list));
    if (!list)
        return NULL;

    list->list   = g_ptr_array_new();
    list->sorted = 1;

    return list;
}

query_instance_list_t *query_instance_list_clone(const query_instance_list_t *list,
                                                 const query_type_list_t     *query_type_list) {

    query_instance_list_t *clone = malloc(sizeof(query_instance_list_t));
    if (!clone)
        return NULL;

    clone->sorted = list->sorted;
    clone->list   = g_ptr_array_sized_new(list->list->len);

    for (guint i = 0; i < list->list->len; ++i) {
        query_instance_t *ins =
            query_instance_clone(g_ptr_array_index(list->list, i), query_type_list);
        if (!ins) { /* Allocation error */
            for (guint j = 0; j < i; ++j) {
                query_instance_free(g_ptr_array_index(clone->list, j), query_type_list);
            }

            g_ptr_array_unref(clone->list);
            free(clone);
            return NULL;
        } else {
            g_ptr_array_add(clone->list, ins);
        }
    }

    return clone;
}

void query_instance_list_add(query_instance_list_t   *list,
                             const query_instance_t  *query,
                             const query_type_list_t *query_type_list) {

    query_instance_t *clone = query_instance_clone(query, query_type_list);
    g_ptr_array_add(list->list, clone);
    list->sorted = 0;
}

/** @brief Compares two query instances to order them by type. */
gint __query_instance_list_compare(gconstpointer a, gconstpointer b) {
    ssize_t crit1 = (ssize_t) query_instance_get_type(*(const query_instance_t *const *) a) -
                    (ssize_t) query_instance_get_type(*(const query_instance_t *const *) b);
    if (crit1)
        return crit1;

    ssize_t crit2 =
        (ssize_t) query_instance_get_number_in_file(*(const query_instance_t *const *) a) -
        (ssize_t) query_instance_get_number_in_file(*(const query_instance_t *const *) b);
    return crit2;
}

int query_instance_list_iter_types(query_instance_list_t                  *list,
                                   query_instance_list_iter_types_callback callback,
                                   void                                   *user_data) {

    /* TODO - check if this works */

    if (!list->sorted) {
        g_ptr_array_sort(list->list, __query_instance_list_compare);
        list->sorted = 1;
    }

    if (list->list->len == 0) /* Don't fail on edge case */
        return 0;

    query_instance_t *instance = g_ptr_array_index(list->list, 0);

    size_t current_set_start = 0;
    size_t current_set_type  = query_instance_get_type(instance); /* First instance */
    size_t current_set_count = 1;

    for (size_t i = 1; i < list->list->len; ++i) {
        instance    = g_ptr_array_index(list->list, i);
        size_t type = query_instance_get_type(instance);

        if (type == current_set_type) {
            current_set_count++;
        } else {
            if (current_set_count > 0) {
                const query_instance_t *const *current_set =
                    (const query_instance_t *const *) &g_ptr_array_index(list->list,
                                                                         current_set_start);
                int cb_ret = callback(user_data, current_set, current_set_count);
                if (cb_ret)
                    return cb_ret;
            }

            current_set_count = 1;
            current_set_start = i;
            current_set_type  = type;
        }
    }

    const query_instance_t *const *current_set =
        (const query_instance_t *const *) &g_ptr_array_index(list->list, current_set_start);
    int cb_ret = callback(user_data, current_set, current_set_count);
    if (cb_ret)
        return cb_ret;

    return 0;
}

int query_instance_list_iter(query_instance_list_t            *list,
                             query_instance_list_iter_callback callback,
                             void                             *user_data) {
    if (!list->sorted) {
        g_ptr_array_sort(list->list, __query_instance_list_compare);
        list->sorted = 1;
    }

    for (size_t i = 0; i < list->list->len; ++i) {
        int retval = callback(user_data, g_ptr_array_index(list->list, i));
        if (retval)
            return retval;
    }

    return 0;
}

size_t query_instance_list_get_length(const query_instance_list_t *list) {
    return list->list->len;
}

void query_instance_list_free(query_instance_list_t   *list,
                              const query_type_list_t *query_type_list) {
    for (size_t i = 0; i < list->list->len; ++i)
        query_instance_free(g_ptr_array_index(list->list, i), query_type_list);
    g_ptr_array_unref(list->list);
    free(list);
}
