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
 * @file  test_diff_output.h
 * @brief Formatter of information in a ::test_diff_t for output to the user.
 *
 * @anchor test_diff_output_example
 * ### Example
 *
 * See test.c to know how to perform this test and how to print its results.
 */

#ifndef TEST_DIFF_OUTPUT_H
#define TEST_DIFF_OUTPUT_H

#include <stdio.h>

#include "testing/test_diff.h"

/**
 * @brief Prints the result of a directory difference test to a file stream.
 *
 * @param output Stream where to output formatted data to.
 * @param diff   Difference test results to be printed.
 */
void test_diff_output_print(FILE *output, const test_diff_t *diff);

#endif
