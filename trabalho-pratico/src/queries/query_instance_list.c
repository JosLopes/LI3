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
    query_instance_list_t *const list = malloc(sizeof(query_instance_list_t));
    if (!list)
        return NULL;

    list->list   = g_ptr_array_new_with_free_func((GDestroyNotify) query_instance_free);
    list->sorted = 1;
    return list;
}

/** @brief `GCopyFunc` to copy a ::query_instance_t. */
gpointer __query_instance_list_copy_query_instance(gconstpointer instance, gpointer user_data) {
    /* TODO - check if this works */
    (void) user_data;
    return query_instance_clone(instance);
}

query_instance_list_t *query_instance_list_clone(const query_instance_list_t *list) {
    query_instance_list_t *const clone = malloc(sizeof(query_instance_list_t));
    if (!clone)
        return NULL;

    clone->sorted = list->sorted;
    clone->list   = g_ptr_array_copy(list->list, __query_instance_list_copy_query_instance, NULL);
    return clone;
}

int query_instance_list_add(query_instance_list_t *list, const query_instance_t *query) {
    query_instance_t *const clone = query_instance_clone(query);
    if (!clone)
        return 1;

    g_ptr_array_add(list->list, clone);
    list->sorted = 0;
    return 0;
}

/** @brief Compares two query instances to order them by type. */
gint __query_instance_list_compare(gconstpointer a, gconstpointer b) {
    const ssize_t crit1 = (ssize_t) query_instance_get_type(*(const query_instance_t *const *) a) -
                          (ssize_t) query_instance_get_type(*(const query_instance_t *const *) b);
    if (crit1)
        return crit1;

    const ssize_t crit2 =
        (ssize_t) query_instance_get_line_in_file(*(const query_instance_t *const *) a) -
        (ssize_t) query_instance_get_line_in_file(*(const query_instance_t *const *) b);
    return crit2;
}

int query_instance_list_iter_types(query_instance_list_t                  *list,
                                   query_instance_list_iter_types_callback callback,
                                   void                                   *user_data) {
    if (!list->sorted) {
        g_ptr_array_sort(list->list, __query_instance_list_compare);
        list->sorted = 1;
    }

    if (list->list->len == 0) /* Don't fail on edge case */
        return 0;

    query_instance_t *instance = g_ptr_array_index(list->list, 0);

    size_t              current_set_start = 0;
    const query_type_t *current_set_type  = query_instance_get_type(instance); /* First instance */
    size_t              current_set_count = 1;

    for (size_t i = 1; i < list->list->len; ++i) {
        instance                 = g_ptr_array_index(list->list, i);
        const query_type_t *type = query_instance_get_type(instance);

        if (type == current_set_type) {
            current_set_count++;
        } else {
            if (current_set_count > 0) {
                const query_instance_t *const *current_set =
                    (const query_instance_t *const *) &g_ptr_array_index(list->list,
                                                                         current_set_start);

                const int cb_ret = callback(user_data, current_set_count, current_set);
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
    const int cb_ret = callback(user_data, current_set_count, current_set);
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
        const int retval = callback(user_data, g_ptr_array_index(list->list, i));
        if (retval)
            return retval;
    }
    return 0;
}

size_t query_instance_list_get_length(const query_instance_list_t *list) {
    return list->list->len;
}

void query_instance_list_free(query_instance_list_t *list) {
    g_ptr_array_unref(list->list);
    free(list);
}
