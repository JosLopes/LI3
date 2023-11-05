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
 * @file  dataset_loader.c
 * @brief Implementation of methods in include/dataset/dataset_loader.h
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_loader_examples).
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include "dataset/dataset_loader.h"
#include "dataset/flights_loader.h"
#include "dataset/passengers_loader.h"
#include "dataset/reservations_loader.h"
#include "dataset/users_loader.h"

/**
 * @struct dataset_loader
 * @brief Data needed for a dataset loader to work.
 *
 * @var dataset_loader::users
 *     @brief File containing the dataset's users.
 * @var dataset_loader::users_errors
 *     @brief File where invalid users will be stored.
 * @var dataset_loader::flights
 *     @brief File containing the dataset's flights.
 * @var dataset_loader::flights_errors
 *     @brief File where invalid flights will be stored.
 * @var dataset_loader::passengers
 *     @brief File containing the dataset's user-flight relationships.
 * @var dataset_loader::passengers_errors
 *     @brief File where invalid user-flight relationships will be store.
 * @var dataset_loader::reservations
 *     @brief File containing the dataset's hotel reservations.
 * @var dataset_loader::reservations_errors
 *     @brief File where invalid hotel reservations will be stored.
 * @var dataset_loader::database
 *     @brief Database being modified.
 */
struct dataset_loader {
    /* clang-format off */
    FILE *users,        *users_errors;
    FILE *flights,      *flights_errors;
    FILE *passengers,   *passengers_errors;
    FILE *reservations, *reservations_errors;
    /* clang-format on */
    database_t *database;
};

/**
 * @brief Opens both files (data and errors) needed to read a part of the dataset.
 * @details Auxiliary method for ::dataset_loader_load.
 *
 * @param path Path (directory) of the dataset.
 * @param type Type of the file to be read (`"users"`, `"flights"`, `"passengers"` or
 *             `"reservations"`).
 * @param data_file   Where to place the data file stream.
 * @param errors_file Where to place the errors file stream.
 *
 * @retval 0 Success to open files.
 * @retval 1 Failure to open at least one of the files.
 */
int __dataset_loader_load_open_files_type(const char *path,
                                          const char *type,
                                          FILE      **data_file,
                                          FILE      **errors_file) {

    char filepath[PATH_MAX];
    sprintf(filepath, "%s/%s.csv", path, type);

    FILE *open_data_file = fopen(filepath, "r");
    if (!open_data_file)
        return 1;
    *data_file = open_data_file;

    sprintf(filepath, "Resultados/%s_errors.csv", type);
    FILE *open_errors_file = fopen(filepath, "w");
    if (!open_data_file)
        return 1;
    *errors_file = open_errors_file;

    return 0;
}

/**
 * @brief Tries to close all file streams in a ::dataset_loader.
 * @param loader Loader to have all its file streams closed.
 */
void __dataset_loader_load_close_all_files(dataset_loader_t *loader) {
    /* No need for error handling, as these errors shouldn't be considered a loader failure */

    /* clang-format off */
    if (loader->users)               fclose(loader->users);
    if (loader->users_errors)        fclose(loader->users_errors);
    if (loader->flights)             fclose(loader->flights);
    if (loader->flights_errors)      fclose(loader->flights_errors);
    if (loader->passengers)          fclose(loader->passengers);
    if (loader->passengers_errors)   fclose(loader->passengers_errors);
    if (loader->reservations)        fclose(loader->reservations);
    if (loader->reservations_errors) fclose(loader->reservations_errors);
    /* clang-format on */
}

int dataset_loader_load(database_t *database, const char *path) {
    dataset_loader_t loader = {0};
    loader.database         = database;

    /* Try to create "Resultados" directory if it doesn't exist */
    int mkdir_res = mkdir("Resultados", 0755);
    if (mkdir_res && errno != EEXIST) {
        return 1; /* Failure to create directory */
    }

    /* Open data and error files */
    const char *types[4]        = {"users", "flights", "passengers", "reservations"};
    FILE      **data_files[4]   = {&loader.users,
                                   &loader.flights,
                                   &loader.passengers,
                                   &loader.reservations};
    FILE      **errors_files[4] = {&loader.users_errors,
                                   &loader.flights_errors,
                                   &loader.passengers_errors,
                                   &loader.reservations_errors};

    for (int i = 0; i < 4; ++i) {
        int fail =
            __dataset_loader_load_open_files_type(path, types[i], data_files[i], errors_files[i]);

        if (fail) {
            __dataset_loader_load_close_all_files(&loader);
            return 1;
        }
    }

    /* Load dataset */
    users_loader_load(&loader, loader.users);
    flights_loader_load(&loader, loader.flights);
    reservations_loader_load(&loader, loader.reservations);
    passengers_loader_load(&loader, loader.reservations);

    __dataset_loader_load_close_all_files(&loader);
    return 0;
}

database_t *dataset_loader_get_database(const dataset_loader_t *loader) {
    return loader->database;
}

void dataset_loader_report_users_error(dataset_loader_t *loader, const char *error_line) {
    fprintf(loader->users_errors, "%s\n", error_line);
}

void dataset_loader_report_flights_error(dataset_loader_t *loader, const char *error_line) {
    fprintf(loader->flights_errors, "%s\n", error_line);
}

void dataset_loader_report_passengers_error(dataset_loader_t *loader, const char *error_line) {
    fprintf(loader->passengers_errors, "%s\n", error_line);
}

void dataset_loader_report_reservations_error(dataset_loader_t *loader, const char *error_line) {
    fprintf(loader->reservations_errors, "%s\n", error_line);
}
