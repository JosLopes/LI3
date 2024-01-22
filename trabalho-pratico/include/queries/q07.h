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
 * @file  q07.h
 * @brief A query to that lists the top N airports by median of departure delays.
 *
 * ### Examples
 *
 * ```text
 * 7 10
 * 7 20
 * 7F 10
 * 7F 20
 * ```
 */

#ifndef Q07_H
#define Q07_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of queries of type 7.
 * @details This is done automatically in [query_type_list](@ref query_type_list.c).
 * @return  On success, a pointer to a ::query_type_t that must be deleted with ::query_type_free,
 *          or `NULL` allocation on failure.
 */
query_type_t *q07_create(void);

#endif
