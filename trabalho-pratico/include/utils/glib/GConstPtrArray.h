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
 * @file    GConstPtrArray.h
 * @brief   A wrapper over a `GPtrArray` but for `const` pointers.
 * @details Available methods constitute a subset of the methods provided by `glib`, and they have
 *          the same behavior. There are many online documentation sources and the following is only
 *          an example: https://libsoup.org/glib/glib-Pointer-Arrays.html
 */

#ifndef G_CONST_PTR_ARRAY_H
#define G_CONST_PTR_ARRAY_H

#include <glib.h>

/**
 * @brief   A dynamic array of constant pointers.
 * @details The underlying `struct` doesn't actually exist. A pointer to a ::GConstPtrArray will
 *          merely be casted to a `GPtrArray *` during operations, but there will be no implicit
 *          casts this way. Also, the pointer cast (as opposed to a real structure) will avoid
 *          memory indirection.
 */
typedef struct GConstPtrArray GConstPtrArray;

/**
 * @brief   Creates a new dynamic array of constant pointers.
 * @details See https://libsoup.org/glib/glib-Pointer-Arrays.html#g-ptr-array-new
 * @return  A pointer to a ::GConstPtrArray.
 */
GConstPtrArray *g_const_ptr_array_new(void);

/**
 * @brief   Adds a pointer to the end of a pointer array.
 * @details See https://libsoup.org/glib/glib-Pointer-Arrays.html#g-ptr-array-add
 *
 * @param array The array to add @p data to.
 * @param data  Pointer to be added to the end of @p array.
 */
void g_const_ptr_array_add(GConstPtrArray *array, gconstpointer data);

/**
 * @brief   Returns the pointer at a given index of a pointer array.
 * @details See https://libsoup.org/glib/glib-Pointer-Arrays.html#g-ptr-array-index
 *
 * @param array The array to get a pointer from.
 * @param index The index of the pointer to return.
 *
 * @return The pointer resulting from @p array indexed at @p index.
 */
gconstpointer g_const_ptr_array_index(const GConstPtrArray *array, guint index);

/**
 * @brief   Returns the length of an array of constant pointers.
 * @details There is no equivalent to this method in `glib`, as the field `len` is directly
 *          accessible from a `GPtrArray`. However, that isn't the case in a ::GConstPtrArray.
 * @return  The lenght of @p array.
 */
guint g_const_ptr_array_get_length(const GConstPtrArray *array);

/**
 * @brief   Sorts an array of constant pointers.
 * @details See https://libsoup.org/glib/glib-Pointer-Arrays.html#g-ptr-array-sort
 *
 * @param array        Array to be sorted.
 * @param compare_func Method used to compare between two given pointers.
 */
void g_const_ptr_array_sort(GConstPtrArray *array, GCompareFunc compare_func);

/**
 * @brief   Decrements the reference count of a dynamic array of constant pointers.
 * @details See https://libsoup.org/glib/glib-Pointer-Arrays.html#g-ptr-array-unref
 * @param   array Array to have its reference count decremented.
 */
void g_const_ptr_array_unref(GConstPtrArray *array);

#endif
