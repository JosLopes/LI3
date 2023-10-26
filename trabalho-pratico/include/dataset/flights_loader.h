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
 * @file    flights_loader.h
 * @brief   Module to load all the flights in a dataset into the database.
 */

#ifndef FLIGHTS_LOADER_H
#define FLIGHTS_LOADER_H

#include <stdio.h>

#include "dataset/dataset_loader.h"

/**
 * @brief Parses a `flights.csv` dataset file.
 *
 * @param dataset_loader Dataset loader, for database modification and error reporting.
 * @param stream         File stream with flight data to be loaded.
 */
void flights_loader_load(dataset_loader_t *dataset_loader, FILE *stream);

#endif
