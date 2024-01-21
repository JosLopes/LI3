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
 * @file  GConstPtrArray.c
 * @brief Implementation of methods in include/utils/glib/GConstPtrArray.h
 */

#include "utils/glib/GConstPtrArray.h"

GConstPtrArray *g_const_ptr_array_new(void) {
    return (GConstPtrArray *) g_ptr_array_new();
}

void g_const_ptr_array_add(GConstPtrArray *array, gconstpointer data) {
    g_ptr_array_add((GPtrArray *) array, (gpointer) data);
}

gconstpointer g_const_ptr_array_index(const GConstPtrArray *array, guint index) {
    return g_ptr_array_index((GPtrArray *) array, index);
}

guint g_const_ptr_array_get_length(const GConstPtrArray *array) {
    return ((const GPtrArray *) array)->len;
}

void g_const_ptr_array_sort(GConstPtrArray *array, GCompareFunc compare_func) {
    g_ptr_array_sort((GPtrArray *) array, compare_func);
}

void g_const_ptr_array_unref(GConstPtrArray *array) {
    g_ptr_array_unref((GPtrArray *) array);
}
