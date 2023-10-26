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
 * @file    dataset_loader.h
 * @brief   Module to load all the files in a dataset into the database.
 * @details A dataset, as specified in the project's requirements, is constituted by the following
 *          files:
 *
 *          - `users.csv`;
 *          - `flights.csv`;
 *          - `passengers.csv`;
 *          - `reservations.csv`.
 */

#ifndef DATASET_LOADER_H
#define DATASET_LOADER_H

#include "database/database.h"

/**
 * @brief Data for a dataset loader.
 * @details To be used only in dataset file parsers.
 */
typedef struct dataset_loader dataset_loader_t;

/**
 * @brief Parses a dataset in @p path and stores the data in @p database.
 *
 * @param database Database where to store the dataset data in.
 * @param path     Path to the directory containing the dataset
 *
 * @retval 0 Success
 * @retval 1 Fatal failure (this loader only fatally fails on IO errors).
 */
int dataset_loader_load(database_t *database, const char *path);

/**
 * @brief Gets the database being modified by a dataset loader.
 * @param loader Dataset loader modifying the database.
 * @return Database being modifier by @p loader.
 */
database_t *dataset_loader_get_database(const dataset_loader_t *loader);

/**
 * @brief Writes a line to the `users_errors.csv` file.
 * @param loader Dataset loader that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_loader_report_users_error(dataset_loader_t *loader, const char *error_line);

/**
 * @brief Writes a line to the `flights_errors.csv` file.
 * @param loader Dataset loader that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_loader_report_flights_error(dataset_loader_t *loader, const char *error_line);

/**
 * @brief Writes a line to the `passengers_errors.csv` file.
 * @param loader Dataset loader that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_loader_report_passengers_error(dataset_loader_t *loader, const char *error_line);

/**
 * @brief Writes a line to the `reservations_errors.csv` file.
 * @param loader Dataset loader that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_loader_report_reservations_error(dataset_loader_t *loader, const char *error_line);

#endif
