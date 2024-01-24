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
 * @file  query_type_list.c
 * @brief Implementation of methods in include/queries/query_type_list.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_type_list_example).
 */

#include "queries/query_type_list.h"

#include "queries/q01.h"
#include "queries/q02.h"
#include "queries/q03.h"
#include "queries/q04.h"
#include "queries/q05.h"
#include "queries/q06.h"
#include "queries/q07.h"
#include "queries/q08.h"
#include "queries/q09.h"
#include "queries/q10.h"

/**
 * @brief   List of all known queries.
 * @details Shall not be modified apart from its creation. It's not constant because it requires
 *          run-time initialization. This global variable is justified for the following reasons:
 *
 *          -# It's not modified (no mutable global state);
 *          -# It's not directly exposed to other modules (very limited scope);
 *          -# It's a list of `vtable`s (dispatch tables). In other languages (such as C++), it'd be
 *             global;
 *          -# It'd be very inefficient for every polymorphic object to have its own copy of its
 *             `vtable`.
 */
query_type_t *__query_type_list[QUERY_TYPE_LIST_COUNT];

/** @brief Automatically initializes ::__query_type_list when the program starts. */
void __attribute__((constructor)) __query_type_list_create(void) {
    query_type_t *(*const constructors[QUERY_TYPE_LIST_COUNT])(void) = {q01_create,
                                                                        q02_create,
                                                                        q03_create,
                                                                        q04_create,
                                                                        q05_create,
                                                                        q06_create,
                                                                        q07_create,
                                                                        q08_create,
                                                                        q09_create,
                                                                        q10_create};
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        __query_type_list[i] = constructors[i]();
}

/** @brief Automatically `free`s ::__query_type_list when the program terminates. */
void __attribute__((destructor)) __query_type_list_free(void) {
    for (size_t i = 0; i < QUERY_TYPE_LIST_COUNT; ++i)
        query_type_free(__query_type_list[i]);
}

const query_type_t *query_type_list_get_by_index(size_t index) {
    if (1 <= index && index <= QUERY_TYPE_LIST_COUNT)
        return __query_type_list[index - 1];
    return NULL;
}
