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
 * @file  database.c
 * @brief Implementation of methods in include/database/database.h
 */

#include <stdlib.h>

#include "database/database.h"

/**
 * @struct database
 * @brief  A collection of the managers of different entities.
 *
 * @var database::placeholder
 *     @brief Temporary variable, as C structs cannot be empty.
 */
struct database {
    int placeholder;
};

database_t *database_create(void) {
    return malloc(sizeof(struct database));
}

void database_free(database_t *database) {
    free(database);
}
