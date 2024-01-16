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
 * @file    GConstKeyHashTable.h
 * @brief   A wrapper over a `GHashTable` where table keys are `const` pointers.
 * @details Available methods constitute a subset of the methods provided by `glib`, and they have
 *          the same behavior. There are many online documentation sources and the following is only
 *          an example: https://libsoup.org/glib/glib-Hash-Tables.html
 */

#ifndef G_CONST_KEY_HASH_TABLE_H
#define G_CONST_KEY_HASH_TABLE_H

#include <glib.h>

/**
 * @brief   A hash table where keys can be pointers to constant values.
 * @details The underlying `struct` doesn't actually exist. A pointer to a ::GConstKeyHashTable will
 *          merely be casted to a `GHashTable *` during operations, but there will be no implicit
 *          casts this way. Also, the pointer cast (as opposed to a real structure) will avoid
 *          memory hops.
 */
typedef struct GConstKeyHashTable GConstKeyHashTable;

/**
 * @brief   Creates a new hash table where keys are pointers to constant values.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-new
 *
 * @param hash_func      Function called for hashing a key.
 * @param key_equal_func Function called for comparing keys.
 *
 * @return A pointer to a ::GConstKeyHashTable. The program may be killed on allocation failure.
 */
GConstKeyHashTable *g_const_key_hash_table_new(GHashFunc hash_func, GEqualFunc key_equal_func);

/**
 * @brief   Inserts a key-value pair into a hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-insert
 *
 * @param hash_table Table where to insert the key-value pair.
 * @param key        Key to be inserted into @p hash_table.
 * @param value      Value to be related to @p key in @p hash_table.
 *
 * @return `TRUE` if the key did not exist yet. The program may be killed on allocation failure.
 */
gboolean g_const_key_hash_table_insert(GConstKeyHashTable *hash_table,
                                       gconstpointer       key,
                                       gpointer            value);

/**
 * @brief   Gets the value a key is associated to in an hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-lookup
 *
 * @param hash_table Table where to perform the lookup.
 * @param key        Key to be searched for.
 *
 * @return The value @p key is associated to, or `NULL` if that key is not in @p hash_table.
 */
gpointer g_const_key_hash_table_lookup(GConstKeyHashTable *hash_table, gconstpointer key);

/**
 * @brief   Decrements the reference count of a hash table.
 * @details See https://libsoup.org/glib/glib-Hash-Tables.html#g-hash-table-unref
 * @param   hash_table Hash table to have its reference count decremented.
 */
void g_const_key_hash_table_unref(GConstKeyHashTable *hash_table);

#endif
