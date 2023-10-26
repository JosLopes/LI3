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
 * @file    database.h
 * @brief   Collection of managers of the different entities.
 * @details Its contents are to be created with the ::dataset_loader_t.
 */

#ifndef DATABASE_H
#define DATABASE_H

/**
 * @brief A database is a collection of the managers of different entities.
 */
typedef struct database database_t;

/**
 * @brief   Instantiates a new ::database_t.
 * @details The returned value is owned by the called and should be `free`'d with ::database_free.
 * @return  The new database, or `NULL` on failure.
 */
database_t *database_create(void);

/**
 * @brief Frees memory used by a database.
 * @param database Database whose memory is to be `free`'d.
 */
void database_free(database_t *database);

#endif
