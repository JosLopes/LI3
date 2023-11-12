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
 *     @brief If the list is sorted. When performing a lookup, the array will be sorted so that
 *            this becomes `1`.
 */
struct query_instance_list {
    GArray *list;
    int     sorted;
};

query_instance_list_t *query_instance_list_create(void) {
    query_instance_list_t *list = malloc(sizeof(struct query_instance_list));
    if (!list)
        return NULL;

    list->list   = g_array_new(FALSE, FALSE, query_instance_sizeof());
    list->sorted = 1;

    return list;
}

void query_instance_list_add(query_instance_list_t *list, query_instance_t *query) {
    g_array_append_vals(list->list, query, 1);
    list->sorted = 0;
}

/** @brief Compares two query instances to order them by type. */
gint __query_instance_list_compare(gconstpointer a, gconstpointer b) {
    ssize_t crit1 = (ssize_t) query_instance_get_type((const query_instance_t *) a) -
                    (ssize_t) query_instance_get_type((const query_instance_t *) b);
    if (crit1)
        return crit1;

    ssize_t crit2 = (ssize_t) query_instance_get_number_in_file((const query_instance_t *) a) -
                    (ssize_t) query_instance_get_number_in_file((const query_instance_t *) b);
    return crit2;
}

int query_instance_list_iter_types(query_instance_list_t                  *list,
                                   query_instance_list_iter_types_callback callback,
                                   void                                   *user_data) {

    if (!list->sorted) {
        g_array_sort(list->list, __query_instance_list_compare);
        list->sorted = 1;
    }

    if (list->list->len == 0) /* Don't fail on edge case */
        return 0;

    query_instance_t *instance = (query_instance_t *) list->list->data;
    size_t            i        = 0;

    query_instance_t *current_set       = instance;
    size_t            current_set_type  = query_instance_get_type(instance); /* First instance */
    size_t            current_set_count = 1;
    while (i < list->list->len) {
        instance = (query_instance_t *) ((uint8_t *) instance + query_instance_sizeof());
        i++;

        if (i == list->list->len || query_instance_get_type(instance) != current_set_type) {
            if (current_set_count > 0) {
                int cb_ret = callback(user_data, current_set, current_set_count);
                if (cb_ret)
                    return cb_ret;
            }

            current_set = instance;
            if (i != list->list->len)
                current_set_type = query_instance_get_type(instance);
            current_set_count = 1;
        } else {
            current_set_count++;
        }
    }

    return 0;
}

int query_instance_list_iter(query_instance_list_t            *list,
                             query_instance_list_iter_callback callback,
                             void                             *user_data) {

    if (!list->sorted) {
        g_array_sort(list->list, __query_instance_list_compare);
        list->sorted = 1;
    }

    for (size_t i = 0; i < list->list->len; ++i) {
        query_instance_t *instance =
            (query_instance_t *) ((uint8_t *) list->list->data + i * query_instance_sizeof());

        int retval = callback(user_data, instance);
        if (retval)
            return retval;
    }

    return 0;
}

size_t query_instance_list_get_length(query_instance_list_t *list) {
    return list->list->len;
}

void query_instance_list_free(query_instance_list_t *list, query_type_list_t *query_type_list) {
    for (size_t i = 0; i < list->list->len; ++i) {
        query_instance_t *instance =
            (query_instance_t *) (list->list->data + i * query_instance_sizeof());
        query_instance_pooled_free(instance, query_type_list);
    }

    g_array_free(list->list, TRUE);
    free(list);
}

void query_instance_list_free_no_internals(query_instance_list_t *list) {
    g_array_free(list->list, TRUE);
    free(list);
}
