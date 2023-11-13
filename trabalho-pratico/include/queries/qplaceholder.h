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
 * @file    qplaceholder.h
 * @brief   A placeholder query.
 * @details TODO - Remove when all queries are done
 */

#ifndef QPLACEHOLDER_H
#define QPLACEHOLDER_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of this placeholder query.
 * @details This is done automatically in ::query_type_list_init.
 * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
 */
query_type_t *qplaceholder_create(void);

#endif
