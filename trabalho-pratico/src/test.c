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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <glib-2.0/glib.h>
#include <stdio.h>

/**
 * @brief A `GHFunc` for iterating through `HashTable` entries.
 * @param key Dictionary key.
 * @param value Dictionary object.
 */
void iter(gpointer key, gpointer value, gpointer user_data) {
    (void) user_data;

    printf("%s é capital da %s\n", (const char *) value, (const char *) key);
}

/**
 * @brief The entry point to the test program.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    GHashTable *dict = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(dict, "Portugal", "Lisboa");
    g_hash_table_insert(dict, "Espanha", "Madrid");
    g_hash_table_insert(dict, "França", "Paris");

    g_hash_table_foreach(dict, iter, NULL);

    g_hash_table_destroy(dict);
    return 0;
}
