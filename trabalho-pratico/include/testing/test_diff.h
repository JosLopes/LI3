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
 * @file    test_diff.h
 * @brief   Information about differences between generated and expected program output.
 * @details Compares two directories.
 *
 * @anchor test_diff_example
 * ### Example
 *
 * See test.c to know how to perform this test and how to print its results using
 * test_diff_output.h.
 */

#ifndef TEST_DIFF_H
#define TEST_DIFF_H

/** @brief Differences between generated and expected program output. */
typedef struct test_diff test_diff_t;

/**
 * @brief Generates the difference between two directories.
 *
 * @param results  Directory where the program's output was placed into.
 * @param expected Directory containing expected program results.
 *
 * @return A valid pointer to a ::test_diff_t, or `NULL` in case of either allocation or IO error.
 *
 * #### Examples
 * See [the header file's documentation](@ref test_diff_example).
 */
test_diff_t *test_diff_create(const char *results, const char *expected);

/**
 * @brief Creates a deep clone of @p diff.
 * @param diff Difference test results to be cloned.
 */
test_diff_t *test_diff_clone(const test_diff_t *diff);

/**
 * @brief  Gets the list of files present in the generated results but not in the expected ones.
 * @param  diff Test results to get the files from.
 * @param  n    Where to write the number of files to.
 * @return Pointers to the file names.
 */
const char *const *test_diff_get_extra_files(const test_diff_t *diff, size_t *n);

/**
 * @brief  Gets the list of files present in the expected results but not in the generated ones.
 * @param  diff Test results to get the files from.
 * @param  n    Where to write the number of files to.
 * @return Pointers to the file names.
 */
const char *const *test_diff_get_missing_files(const test_diff_t *diff, size_t *n);

/**
 * @brief Gets the lines where differences were found between files.
 *
 * @param diff         Test results to get the files and errors from.
 * @param common_files Where to output the list of files to.
 * @param errors       First line where a difference was found for each file. `0` means no error and
 *                     `-1` means an IO occurred.
 *
 * @return The number of values in both @p common_files and @p errors.
 */
size_t test_diff_get_common_file_errors(const test_diff_t  *diff,
                                        const char *const **common_files,
                                        ssize_t const     **errors);

/**
 * @brief Frees memory allocated by ::test_diff_create.
 * @param diff Value returned by ::test_diff_create.
 *
 * #### Examples
 * See [the header file's documentation](@ref test_diff_example).
 */
void test_diff_free(test_diff_t *diff);

#endif
