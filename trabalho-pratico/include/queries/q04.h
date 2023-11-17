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
  * @file    q04.h
  * @brief   List hotel reservations, ordered by start date (from most recent to oldest).
  * @details If two reservations have the same date, the reservation identifier must
  *          be used as a tiebreaker (in ascending order).
  *
  * ### Examples
  *
  * ```text
  * 4 HTL1001
  * 4 HTL1002
  * 4 HTL1003
  * 4F HTL1001
  * 4F HTL1002
  * 4F HTL1003
  * ```
  */

#ifndef Q04_H
#define Q04_H

#include "queries/query_type.h"

/**
  * @brief   Initializes the definition of the fourth query.
  * @details This is done automatically in ::query_type_list_create.
  * @return  A pointer to a `malloc`-allocated ::query_type_t on success, or `NULL` on failure.
  */
query_type_t *q04_create(void);

#endif
