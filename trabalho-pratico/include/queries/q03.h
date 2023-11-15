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
 * @file    q03.h
 * @brief   A query to present the averate rating of an hotel, given its identifier.
 *
 * ### Examples
 *
 * ```text
 * 3 HTL1001
 * 3 HTL1002
 * 3 HTL1003
 * 3F HTL1001
 * 3F HTL1002
 * 3F HTL1003
 * ```
 */

#ifndef Q03_H
#define Q03_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of the third query.
 * @details This is done automatically in ::query_type_list_create.
 * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
 */
query_type_t *q03_create(void);

#endif
