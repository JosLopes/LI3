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
 * @file    int_utils.h
 * @brief   Utilities for integers.
 *
 * @anchor int_utils_examples
 * ### Example
 *
 * ```
 * uint64_t parsed;
 * int retcode = int_utils_parse_positive(&parsed, "1732");
 * if (retcode) {
 *     fputs("Failed to parse integer\n", stderr);
 * } else {
 *     printf("Parsed integer is %" PRId64 "\n", parsed);
 * }
 * ```
 *
 * The line `Parsed integer is 1732` is printed.
 */

#ifndef INT_UTILS_H
#define INT_UTILS_H

#include <inttypes.h>

/**
 * @brief Parses an integer, failing on any characters other than decimal digits.
 * @details Unlike `atoi` and `strtol`, this function assures a positive integer and purposely
 *          fails on any whitespace or ``'+'`` / ``'-'`` characters.
 *
 * @param output Where to place the parsed integer. Nothing will be written on failure.
 * @param input  String to be parsed.
 *
 * @retval 0 Parsing success
 * @retval 1 Parsing failure
 *
 * #### Example
 *
 * See [the header file's documentation](@ref int_utils_examples).
 */
int int_utils_parse_positive(uint64_t *output, const char *input);

#endif
