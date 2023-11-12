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
 * @file    query_instance_typedef.h
 * @brief   See query_instance.h.
 * @details This file exists to solve a fake circular dependency, as query_type.h only needs to
 *          know ::query_instance_t is a valid type (to define its callbacks), but does not need to
 *          interact with it (that's left to the callback implementation in q1.h through q10.h).
 */

#ifndef QUERY_INSTANCE_TYPEDEF_H
#define QUERY_INSTANCE_TYPEDEF_H

/**
 * @brief Instance of a query in a query file / inputted by the user.
 */
typedef struct query_instance query_instance_t;

#endif
