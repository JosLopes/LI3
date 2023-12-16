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
 * @file  dataset_error_output.c
 * @brief Implementation of methods in include/dataset/dataset_error_output.h
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_error_output_examples).
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "dataset/dataset_error_output.h"

/**
 * @struct dataset_error_output
 * @brief Collection of file handles for all dataset error files.
 *
 * @var dataset_error_output::users
 *     @brief File where invalid users will be stored.
 * @var dataset_error_output::flights
 *     @brief File where invalid flights will be stored.
 * @var dataset_error_output::passengers
 *     @brief File where invalid user-flight relationships will be stored.
 * @var dataset_error_output::reservations
 *     @brief File where invalid hotel reservations will be stored.
 */
struct dataset_error_output {
    FILE *users;
    FILE *flights;
    FILE *passengers;
    FILE *reservations;
};

dataset_error_output_t *dataset_error_output_create(const char *path) {
    dataset_error_output_t *output = malloc(sizeof(dataset_error_output_t));
    if (!output)
        return NULL;

    if (!path) {
        output->users = output->flights = output->passengers = output->reservations = NULL;
        return output;
    }

    /* Try to create the directory if it doesn't exist */
    int mkdir_res = mkdir(path, 0755);
    if (mkdir_res && errno != EEXIST) {
        free(output);
        return NULL;
    }

    const char *types[4] = {"users", "flights", "passengers", "reservations"};
    FILE      **files[4] = {&output->users,
                            &output->flights,
                            &output->passengers,
                            &output->reservations};

    for (int i = 0; i < 4; ++i) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s_errors.csv", path, types[i]);

        *files[i] = fopen(file_path, "w");

        if (!*files[i]) {
            for (int j = 0; j < i - 1; ++i)
                fclose(*files[i]);

            free(output);
            return NULL;
        }
    }

    return output;
}

void dataset_error_output_report_user_error(dataset_error_output_t *output,
                                            const char             *error_line) {

    if (output->users)
        fprintf(output->users, "%s\n", error_line);
}

void dataset_error_output_report_flight_error(dataset_error_output_t *output,
                                              const char             *error_line) {

    if (output->flights)
        fprintf(output->flights, "%s\n", error_line);
}

void dataset_error_output_report_passenger_error(dataset_error_output_t *output,
                                                 const char             *error_line) {

    if (output->passengers)
        fprintf(output->passengers, "%s\n", error_line);
}

void dataset_error_output_report_reservation_error(dataset_error_output_t *output,
                                                   const char             *error_line) {

    if (output->reservations)
        fprintf(output->reservations, "%s\n", error_line);
}

void dataset_error_output_free(dataset_error_output_t *output) {
    if (!output->users) { /* No output files case. All files will be NULL. */
        free(output);
        return;
    }

    fclose(output->users);
    fclose(output->flights);
    fclose(output->passengers);
    fclose(output->reservations);

    free(output);
}
