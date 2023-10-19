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
 * @file stream_utils.h
 * @brief Contains utility methods for dealing with file streams.
 * 
 * @anchor stream_utils_examples
 * ### Examples
 * 
 * testfile
 * ```
 * Split this
 * text file by
 * 
 * the newline character
 * ```
 * 
 * ```c
 * FILE *fs = fopen("/files/testfile", "r"); // Read-only file, contents remain unchanged.
 * stream_tokenize(fs, '\n', callback, NULL);
 * ```
 * 
 * - `callback("Split this", NULL)`;
 * - `callback("text file by", NULL)`;
 * - `callback("", NULL)`;
 * - `callback("the newline character", NULL)`.
 */

#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

/**
 * @brief Callback method for ::stream_tokenize, called for every token read.
 *
 * @param user_data Pointer provided to ::stream_tokenize, kept from call to call, so that this
 *                  callback can modify the program's state.
 * @param token     The token that was read.
 *
 * @return `0` on success, other value for immediate termination of tokenization.
 */
typedef int (*tokenize_iter_callback_t)(void *user_data, char *token);

/**
 * @brief Splits a **MODIFIABLE** string into tokens, separated by `delimiter`.
 *
 * @param file      File to tokenize. The file is not changed while being tokenized.
 * @param delimiter Character to separate tokens. It won't be part of those tokens.
 * @param callback  Function called for every token read.
 * @param user_data Pointer passed to every call of `callback`, so that it can edit program state.
 *
 * @return `0` on success, otherwise, the return value from @p callback in case it ordered the
 *         tokenization to stop.
 *
 * #### Examples
 * See [the header file's documentation](@ref stream_utils_examples).
 */
int stream_tokenize(FILE                    *file,
                    char                     delimiter,
                    tokenize_iter_callback_t callback,
                    void                    *user_data);

#endif
