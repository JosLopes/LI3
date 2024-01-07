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
 * @file  test_diff_output.c
 * @brief Implementation of methods in include/testing/test_diff_output.h
 *
 * #### Examples
 * See [the header file's documentation](@ref test_diff_output_example).
 */

#include <ctype.h>
#include <string.h>

#include "testing/test_diff_output.h"

/**
 * @brief   Prints a category of errors, all of the same type.
 * @details This method is not great for localization, as many languages have word forms other than
 *          singular and plural.
 *
 * @param output   File where to output the formatted errors to.
 * @param title    Name of the error type.
 * @param strings  Strings containing the errors.
 * @param n        Number of @p strings.
 * @param singular Name of one erroneous object (e.g.: ``"missing file"``)
 * @param plural   Name of multiple erroneous objects (e.g.: ``"missing files"``)
 */
void __test_diff_output_print_category(FILE              *output,
                                       const char        *title,
                                       const char *const *strings,
                                       size_t             n,
                                       const char        *singular,
                                       const char        *plural) {
    /* Format and print colored title */
    fprintf(output, "%s: ", title);

    const char *color_in = "", *color_out = "";
    if (output == stdout) {
        color_out = "\x1b[22;39m";
        if (n == 0)
            color_in = "\x1b[1;32m";
        else
            color_in = "\x1b[1;31m";
    }

    char number[256];
    if (n == 0)
        strcpy(number, "No");
    else
        snprintf(number, 256, "%zu", n);

    const char *noun = (n == 0 || n > 1) ? plural : singular;

    fprintf(output, "%s%s %s\n%s", color_in, number, noun, color_out);

    /* Print actual strings */
    for (size_t i = 0; i < n; ++i) {
        fprintf(output, "    %s\n", strings[i]);
    }
    if (n != 0)
        putchar('\n');
}

void test_diff_output_print(FILE *output, const test_diff_t *diff) {
    if (output == stdout)
        fprintf(output, "\x1b[1;4mEXPECTED RESULTS\x1b[22;24m\n\n");
    else
        fprintf(output, "EXPECTED RESULTS\n\n");

    size_t             n;
    const char *const *extra = test_diff_get_extra_files(diff, &n);
    __test_diff_output_print_category(output, "Extra files", extra, n, "extra file", "extra files");

    const char *const *missing = test_diff_get_missing_files(diff, &n);
    __test_diff_output_print_category(output,
                                      "Missing files",
                                      missing,
                                      n,
                                      "missing file",
                                      "missing files");

    if (n == 0)
        putchar('\n');
}
