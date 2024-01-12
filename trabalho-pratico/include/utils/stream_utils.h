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
 * @file  stream_utils.h
 * @brief Contains utility methods for dealing with file streams.
 *
 * @anchor stream_utils_examples
 * ### Examples
 *
 * Consider the following plain text file, `testfile.txt`:
 *
 * ```
 * Split this
 * text file by
 *
 * the newline character
 * ```
 *
 * The file can be opened, and ::stream_tokenize will read it and tokenize it, calling `callback`
 * for every token.
 *
 * ```c
 * FILE *fs = fopen("testfile.txt", "r"); // Error handling omitted
 * stream_tokenize(fs, '\n', callback, NULL);
 * fclose(fs);
 * ```
 *
 * - `callback("Split this", NULL)`;
 * - `callback("text file by", NULL)`;
 * - `callback("", NULL)`;
 * - `callback("the newline character", NULL)`.
 */

#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

#include "utils/tokenize_iter_callback.h"

/** @brief Value returned by ::stream_tokenize when allocations from `getdelim` fail. */
#define STREAM_TOKENIZE_RET_ALLOCATION_FAILURE -1

#include <stdio.h>

/**
 * @brief Splits a file into tokens, separated by @p delimiter.
 *
 * @param file      File to tokenize.
 * @param delimiter Character to separate tokens. It won't be part of those tokens.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of @p callback, so that it can modify the program's
 *                  state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop. ::STREAM_TOKENIZE_RET_ALLOCATION_FAILURE may also be returned on
 *         allocation failures.
 *
 * #### Examples
 * See [the header file's documentation](@ref stream_utils_examples).
 */
int stream_tokenize(FILE *file, char delimiter, tokenize_iter_callback_t callback, void *user_data);

#endif
