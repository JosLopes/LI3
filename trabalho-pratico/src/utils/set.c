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
 * @file  set.c
 * @brief Implementation of methods in include/utils/set.h
 *
 * ### Examples
 * See [the header file's documentation](@ref set_examples).
 */

#include <glib.h>

#include "utils/set.h"

/**
 * @struct set
 * @brief A set of items.
 *
 * @var set::set_table
 *     @brief Hash table used to implement the set.
 */
struct set {
    GHashTable *set_table;
};

set_t *set_create(set_equal_callback_t equal_func, set_hash_callback_t hash_func) {
    set_t *ret = malloc(sizeof(set_t));
    if (!ret)
        return NULL;

    ret->set_table = g_hash_table_new(hash_func, equal_func);
    return ret;
}

int set_belongs(const set_t *set, const void *element) {
    return g_hash_table_contains(set->set_table, element);
}

void set_add(set_t *set, void *element) {
    g_hash_table_insert(set->set_table, element, NULL);
}

void set_remove(set_t *set, const void *element) {
    g_hash_table_remove(set->set_table, element);
}

int set_iter(const set_t *set, set_iter_callback_t callback, void *user_data) {
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, set->set_table);

    gpointer key;
    while (g_hash_table_iter_next(&iter, &key, NULL)) {
        int cb_ret = callback(user_data, key);
        if (cb_ret)
            return cb_ret;
    }

    return 0;
}

void set_free(set_t *set) {
    g_hash_table_unref(set->set_table);
    free(set);
}
