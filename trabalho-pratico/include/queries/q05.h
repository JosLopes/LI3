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
 * @file  q05.h
 * @brief A query to present all of the flights scheduled to departure, given an origin airport and
 *        a time frame.
 *
 * ### Examples
 *
 * ```text
 * 5 LIS "2021/01/01 00:00:00" "2022/12/31 23:59:59"
 * 5 LIS "2023/01/01 00:00:00" "2023/05/31 23:59:59"
 * 5 AMS "2021/01/01 00:00:00" "2024/01/01 23:59:59"
 * 5F LIS "2021/01/01 00:00:00" "2022/12/31 23:59:59"
 * 5F LIS "2023/01/01 00:00:00" "2023/05/31 23:59:59"
 * 5F AMS "2021/01/01 00:00:00" "2024/01/01 23:59:59"
 * ```
 */

#ifndef Q05_H
#define Q05_H

#include "queries/query_type.h"

/**
 * @brief   Initializes the definition of the fifth query.
 * @details This is done automatically in ::query_type_list_init.
 * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
 */
query_type_t *q05_create(void);

#endif
