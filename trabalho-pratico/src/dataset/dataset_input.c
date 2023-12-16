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
 * @file  dataset_input.c
 * @brief Implementation of methods in include/dataset/dataset_input.h
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_input_examples).
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "dataset/dataset_input.h"
#include "dataset/flights_loader.h"
#include "dataset/passengers_loader.h"
#include "dataset/reservations_loader.h"
#include "dataset/users_loader.h"

/**
 * @struct dataset_input
 * @brief Collection of file handles to the dataset input files.
 *
 * @var dataset_input::users
 *     @brief File containing the dataset's users.
 * @var dataset_input::flights
 *     @brief File containing the dataset's flights.
 * @var dataset_input::passengers
 *     @brief File containing the dataset's user-flight relationships.
 * @var dataset_input::reservations
 *     @brief File containing the dataset's hotel reservations.
 */
struct dataset_input {
    FILE *users;
    FILE *flights;
    FILE *passengers;
    FILE *reservations;
};

dataset_input_t *dataset_input_create(const char *path) {
    dataset_input_t *input = malloc(sizeof(dataset_input_t));
    if (!input)
        return NULL;

    const char *types[4] = {"users", "flights", "passengers", "reservations"};
    FILE **files[4] = {&input->users, &input->flights, &input->passengers, &input->reservations};

    for (int i = 0; i < 4; ++i) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s.csv", path, types[i]);

        *files[i] = fopen(file_path, "r");

        if (!*files[i]) {
            for (int j = 0; j < i - 1; ++i)
                fclose(*files[i]);

            free(input);
            return NULL;
        }
    }

    return input;
}

int dataset_input_load_users(dataset_input_t        *input,
                             dataset_error_output_t *output,
                             database_t             *database) {
    rewind(input->users);
    return users_loader_load(input->users, database, output);
}

int dataset_input_load_flights(dataset_input_t        *input,
                               dataset_error_output_t *output,
                               database_t             *database) {
    rewind(input->flights);
    return flights_loader_load(input->flights, database, output);
}

int dataset_input_load_passengers(dataset_input_t        *input,
                                  dataset_error_output_t *output,
                                  database_t             *database) {
    rewind(input->passengers);
    rewind(input->flights);
    return passengers_loader_load(input->passengers, input->flights, database, output);
}

int dataset_input_load_reservations(dataset_input_t        *input,
                                    dataset_error_output_t *output,
                                    database_t             *database) {
    rewind(input->reservations);
    return reservations_loader_load(input->reservations, database, output);
}

void dataset_input_free(dataset_input_t *input) {
    fclose(input->users);
    fclose(input->flights);
    fclose(input->passengers);
    fclose(input->reservations);

    free(input);
}
