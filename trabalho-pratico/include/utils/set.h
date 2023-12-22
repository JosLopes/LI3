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
 * @file  set.h
 * @brief   A set of items.
 * @details This is a very simple implementation, so many methods expected from a set, such as
 *          intersections or unions, are lacking. The purpose of this module is to simply be used
 *          for belonging checks.
 *
 * @anchor set_examples
 * ### Examples
 *
 * In the following example, a set is created and filled with elements. Then, it is queried about
 * whether some elements belong to it or not. After removing other elements from the set, we iterate
 * over all remaining set elements.
 *
 * ```c
 * int iter(void *user_data, const void *element) {
 *     (void) user_data;
 *     printf("%d is an element of the set\n", GPOINTER_TO_INT(element));
 *     return 0;
 * }
 *
 * int main(int argc, char **argv) {
 *     set_t *set = set_create(g_direct_equal, g_direct_hash);
 *     if (!set)
 *         return 1;
 *
 *     set_add(set, GINT_TO_POINTER(1));
 *     set_add(set, GINT_TO_POINTER(5));
 *     set_add(set, GINT_TO_POINTER(9));
 *
 *     for (int i = 0; i < 10; ++i) {
 *         if (set_belongs(set, GINT_TO_POINTER(i)))
 *             printf("%d belongs to the set\n", i);
 *         else
 *             printf("%d does not belong to the set\n", i);
 *     }
 *
 *     putchar('\n');
 *     set_remove(set, GUINT_TO_POINTER(5));
 *     set_iter(set, iter, NULL);
 *
 *     set_free(set);
 *     return 0;
 * }
 * ```
 *
 * Here's a possible output (there is another possiblity, as a set has no sense of order during
 * iteration):
 *
 * ```text
 * 0 does not belong to the set
 * 1 belongs to the set
 * 2 does not belong to the set
 * 3 does not belong to the set
 * 4 does not belong to the set
 * 5 belongs to the set
 * 6 does not belong to the set
 * 7 does not belong to the set
 * 8 does not belong to the set
 * 9 belongs to the set
 *
 * 1 is an element of the set
 * 9 is an element of the set
 * ```
 */

#ifndef SET_H
#define SET_H

#include <inttypes.h>

/** @brief A set of items. */
typedef struct set set_t;

/**
 * @brief   Function that compares two elements of a set.
 * @details See `GEqualFunc`. Functions of this type (such as `g_str_equal`, `g_direct_equal`, ...)
 *          are compatible with ::set_equal_callback_t.
 *
 * @param a Pointer to first element to be compared.
 * @param b Pointer to second element to be compared.
 *
 * @returns `1` if `a == b`, `0` otherwise.
 */
typedef int (*set_equal_callback_t)(const void *a, const void *b);

/**
 * @brief   Function that hashes an element of a set.
 * @details See `GHashFunc`. Functions of this type (such as `g_str_hash`, `g_direct_hash`, ...)
 *          are compatible with ::set_hash_callback_t.
 *
 * @param element Pointer to value to be hashed.
 *
 * @return Hash of @p x.
 */
typedef uint32_t (*set_hash_callback_t)(const void *element);

/**
 * @brief Function called for every element of a set in ::set_iter.
 *
 * @param element   Element being iterated over.
 * @param user_data Program context, so that this method can modify the program's state.
 *
 * @return `0` to continue iterating, any other value to stop iteration.
 */
typedef int (*set_iter_callback_t)(void *user_data, const void *element);

/**
 * @brief Creates an empty set, implemented as a hash table.
 *
 * @param equal_func Function used to compare elements in the set.
 * @param hash_func  Function used to hash elements in the set.
 *
 * @return A set that must be deleted with ::set_free, or `NULL` on allocation failure.
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
set_t *set_create(set_equal_callback_t equal_func, set_hash_callback_t hash_func);

/**
 * @brief Checks if an element belongs to a set.
 *
 * @param set     Set in which to check for the presence of @p element.
 * @param element Element to be searched form in @p element.
 *
 * @retval 0 @p element does not belong to @p set
 * @retval 1 @p element belongs to @p set
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
int set_belongs(const set_t *set, const void *element);

/**
 * @brief Adds an element to a set.
 *
 * @p set     Set to which @p element will be added.
 * @p element Element to add to @p set.
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
void set_add(set_t *set, void *element);

/**
 * @brief Removes an element from a set.
 *
 * @p set     Set from which @p element will be removed.
 * @p element Element to remove from @p set.
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
void set_remove(set_t *set, const void *element);

/**
 * @brief Iterates over all elements of a set.
 *
 * @param set       Set to iterate over.
 * @param callback  Method to be called for every element of the set.
 * @param user_data Data passed to every @p callback, so that it can edit the program's state.
 *
 * @return The value returned by the last @p callback (this will, of course, be `0` if iteration)
 *         reached its end.
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
int set_iter(const set_t *set, set_iter_callback_t callback, void *user_data);

/**
 * @brief Frees memory allocated by ::set_create.
 * @param set Set allocated by ::set_create.
 *
 * #### Examples
 * See [the header file's documentation](@ref set_examples).
 */
void set_free(set_t *set);

#endif
