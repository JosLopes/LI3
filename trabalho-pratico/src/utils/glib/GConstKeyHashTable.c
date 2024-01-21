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
 * @file  GConstKeyHashTable.c
 * @brief Implementation of methods in include/utils/glib/GConstKeyHashTable.h
 */

#include "utils/glib/GConstKeyHashTable.h"

GConstKeyHashTable *g_const_key_hash_table_new(GHashFunc hash_func, GEqualFunc key_equal_func) {
    return (GConstKeyHashTable *) g_hash_table_new(hash_func, key_equal_func);
}

GConstKeyHashTable *g_const_key_hash_table_new_full(GHashFunc      hash_func,
                                                    GEqualFunc     key_equal_func,
                                                    GDestroyNotify value_destroy_func) {

    return (GConstKeyHashTable *)
        g_hash_table_new_full(hash_func, key_equal_func, NULL, value_destroy_func);
}

gboolean g_const_key_hash_table_insert(GConstKeyHashTable *hash_table,
                                       gconstpointer       key,
                                       gpointer            value) {
    return g_hash_table_insert((GHashTable *) hash_table, (gpointer) key, value);
}

gpointer g_const_key_hash_table_lookup(GConstKeyHashTable *hash_table, gconstpointer key) {
    return g_hash_table_lookup((GHashTable *) hash_table, key);
}

gconstpointer g_const_key_hash_table_const_lookup(const GConstKeyHashTable *hash_table,
                                                  gconstpointer             key) {
    return g_hash_table_lookup((GHashTable *) hash_table, key);
}

void g_const_key_hash_table_unref(GConstKeyHashTable *hash_table) {
    g_hash_table_unref((GHashTable *) hash_table);
}
