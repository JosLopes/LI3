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
 * @file  users_loader.c
 * @brief Implementation of methods in include/dataset/users_loader.h
 */

#include "dataset/users_loader.h"

/** @brief Table header for user errors */
#define USER_LOADER_HEADER                                                                         \
    "id;name;email;phone_number;birth_date;sex;passport;country_code;address;account_creation;"    \
    "pay_method;account_status"

/**
 * @brief Temporary data needed to load a set of users.
 */
typedef struct {
    dataset_loader_t *dataset;
    database_t       *database;
} users_loader_t;

void users_loader_load(dataset_loader_t *dataset_loader, FILE *stream) {
    (void) stream;

    dataset_loader_report_users_error(dataset_loader, USER_LOADER_HEADER);

    /* Test errors */
    dataset_loader_report_users_error(dataset_loader, "User error");
    dataset_loader_report_flights_error(dataset_loader, "Flight error");
    dataset_loader_report_passengers_error(dataset_loader, "Passenger error");
    dataset_loader_report_reservations_error(dataset_loader, "Resevation error");
}
