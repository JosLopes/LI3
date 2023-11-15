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
  * @file    q06.h
  * @brief   List the top N airports with the most passengers, for a given year. Flights with an
  *          estimated departure date in that year must be counted.
  * @details If two airports have the same value, the name of the airport must be used as a
  *          tiebreaker (in ascending order).
  *
  * ### Examples
  *
  * ```text
  * 6 2023 10
  * 6 2022 10
  * 6 2021 10
  * 6F 2023 10
  * 6F 2022 10
  * 6F 2021 10
  * ```
  */

#ifndef Q06_H
#define Q06_H

#include "queries/query_type.h"

/**
  * @brief   Initializes the definition of the fourth query.
  * @details This is done automatically in ::query_type_list_create.
  * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
  */
query_type_t *q06_create(void);

#endif
