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
 * @file    reservations_loader.h
 * @brief   Module to load all the reservations in a dataset into the database.
 */

#ifndef RESERVATIONS_LOADER_H
#define RESERVATIONS_LOADER_H

#include <stdio.h>

#include "dataset/dataset_loader.h"

/**
 * @brief Parses a `reservations.csv` dataset file.
 *
 * @param dataset_loader Dataset loader, for database modification and error reporting.
 * @param stream         File stream with reservation data to be loaded.
 */
void reservations_loader_load(dataset_loader_t *dataset_loader, FILE *stream);

#endif
