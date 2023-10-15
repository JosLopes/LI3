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
 * @file string_utils.h
 * @brief Contains utility methods for dealing with strings.
 *
 * @anchor string_utils_examples
 * ### Examples
 *
 * In this section of examples, `string_const_tokenize` was used, but `string_tokenize` presents
 * the same behavior, with the exception that the @p input must be modifiable.
 *
 * Calling `string_const_tokenize` with the following arguments will result in the following
 * @p callback calls:
 *
 * ```c
 * string_const_tokenize("Split by  spaces", ' ', callback, NULL);
 * ```
 *
 * - `callback("Split", NULL)`;
 * - `callback("by", NULL)`;
 * - `callback("", NULL)`;
 * - `callback("spaces", NULL)`.
 *
 * A complete example of `string_const_tokenize` follows. Suppose we want to calculate the average
 * height of a group of students (in centimeters):
 *
 * ```
 * #include <stdio.h>
 * #include <stdlib.h>
 *
 * #include "utils/string_utils.h"
 *
 * #define STUDENT_HEIGHTS "160,170,182,165"
 *
 * int iter(void *user_data, char *token) {
 *     int *sum_count = (int *) user_data;
 *
 *     int height = atoi(token);
 *     if (height <= 0) {
 *         fprintf(stderr, "Invalid height: \"%s\"\n", token);
 *         return 1;
 *     } else {
 *         sum_count[0] += height; // sum += height;
 *         sum_count[1]++;         // count++;
 *     }
 *
 *     return 0;
 * }
 *
 * int main(void) {
 *     int sum_count[2] = {0, 0};
 *
 *     if (string_const_tokenize(STUDENT_HEIGHTS, ',', iter, sum_count)) {
 *         return 1;
 *     } else {
 *         printf("Average height is: %.2fcm\n", (double) sum_count[0] / sum_count[1]);
 *         return 0;
 *     }
 * }
 * ```
 *
 * Interruption of tokenization is also exemplified here: if one of the heights is invalid
 * (e.g.: `"-180"`), the program won't keep reading tokens and will stop immediately. Also, `main`
 * can see whether tokenization was stopped because of an error (`1` is returned) or not (`0` is
 * returned).
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/**
 * @brief Callback method for ::string_tokenize, called for every token read.
 *
 * @param user_data Pointer provided to ::string_tokenize, kept from call to call, so that this
 *                  callback can modify the program's state.
 * @param token     The token that was read.
 *
 * @return `0` on success, other value for immediate termination of tokenization. It's recommended
 *         that this value is positive, not to risk being confused with
 *         ::STRING_CONST_TOKENIZE_FAILED_MALLOC.
 */
typedef int (*tokenize_iter_callback_t)(void *user_data, char *token);

/**
 * @brief Splits a **MODIFIABLE** string into tokens, separated by `delimiter`.
 *
 * @param input     String to tokenize, that that will be modified for this function to work.
 * @param delimiter Character to separate tokens. It won't be part of those tokens.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of `callback`, so that it can edit program state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_utils_examples).
 */
int string_tokenize(char                    *input,
                    char                     delimiter,
                    tokenize_iter_callback_t callback,
                    void                    *user_data);

/**
 * @brief Value returned by ::string_const_tokenize when `malloc` fails.
 */
#define STRING_CONST_TOKENIZE_FAILED_MALLOC -1

/**
 * @brief See ::string_tokenize, but this method applies to `const` strings.
 *
 * @details The current implementation allocates a writeable buffer and copies over the string
 *          before calling ::string_tokenize, so **it's very inefficient** and should not be used
 *          for large strings.
 *
 * @param input     String to tokenize.
 * @param delimiter Character to separate tokens. It won't be part of those tokens.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of `callback`, so that it can edit program state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop. ::STRING_CONST_TOKENIZE_FAILED_MALLOC is returned on failure to
 *         allocate a writeable buffer the size of @p input.
 *
 * #### Examples
 * See [the header file's documentation](@ref string_utils_examples).
 */
int string_const_tokenize(const char              *input,
                          char                     delimiter,
                          tokenize_iter_callback_t callback,
                          void                    *user_data);

#endif
