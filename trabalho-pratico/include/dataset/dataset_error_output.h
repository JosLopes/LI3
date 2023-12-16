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
 * @file  dataset_error_output.h
 * @brief Module to report dataset errors. See [dataset_loader](@ref dataset_loader.h) for more
 *        information.
 *
 * @anchor dataset_error_output_examples
 * ### Example
 *
 * In order to load a dataset, this module should be used in conjunction with
 * [dataset_input](@ref dataset_input.h). See the source code of ::dataset_loader_load
 * for a good example on how to use both of these modules. Error reporting funtions
 * (`dataset_error_output_report_*_error`) are used in individual file dataset loaders (`*_loader`).
 */

#ifndef DATASET_ERROR_OUTPUT_H
#define DATASET_ERROR_OUTPUT_H

/** @brief Collection of file handles for all dataset error files. */
typedef struct dataset_error_output dataset_error_output_t;

/**
 * @brief Attempts to open all file handles for dataset error files.
 *
 * @param path Path to the directory where to create the error files. If the directory does not
 *             exists, it'll be created aswell. Provide `NULL` for no error output.
 *
 * @return A collection of file handles that must be `free`'d with ::dataset_error_output_free, or
 *         `NULL` on IO error.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_error_output_examples).
 */
dataset_error_output_t *dataset_error_output_create(const char *path);

/**
 * @brief Writes a line to the `users_errors.csv` file.
 * @param error_output ::dataset_error_output_t that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_error_output_report_user_error(dataset_error_output_t *output, const char *error_line);

/**
 * @brief Writes a line to the `flights_errors.csv` file.
 * @param error_output ::dataset_error_output_t that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_error_output_report_flight_error(dataset_error_output_t *output,
                                              const char             *error_line);

/**
 * @brief Writes a line to the `passengers_errors.csv` file.
 * @param error_output ::dataset_error_output_t that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_error_output_report_passenger_error(dataset_error_output_t *output,
                                                 const char             *error_line);

/**
 * @brief Writes a line to the `reservations_errors.csv` file.
 * @param error_output ::dataset_error_output_t that opened the errors file.
 * @param error_line Error to be reported (musn't include ``'\n'``).
 */
void dataset_error_output_report_reservation_error(dataset_error_output_t *output,
                                                   const char             *error_line);

/**
 * @brief Closes all file handles in @p input and `free`s the data structure.
 * @param input Value to be deleted, allocated by ::dataset_error_output_create.
 *
 * #### Example
 * See [the header file's documentation](@ref dataset_error_output_examples).
 */
void dataset_error_output_free(dataset_error_output_t *input);

#endif
