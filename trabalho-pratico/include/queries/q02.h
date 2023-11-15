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
 * @file    q02.h
 * @brief   A query to list flights / reservations related to a user.
 * @details
 *
 * ### Examples
 *
 * ```
 * 2 JéssiTavares910
 * 2 JéssiTavares910 flights
 * 2 JéssiTavares910 reservations
 * 2 DGarcia429
 * 2 LGarcia1208
 * 2 SPinho687
 * 2F JéssiTavares910
 * 2F JéssiTavares910 flights
 * 2F JéssiTavares910 reservations
 * 2F DGarcia429
 * 2F LGarcia1208
 * 2F SPinho687
 * ```
 */

#ifndef Q02_H
#define Q02_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of the query of type 2.
 * @details This is done automatically in ::query_type_list_create.
 * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
 */
query_type_t *q02_create(void);

#endif
