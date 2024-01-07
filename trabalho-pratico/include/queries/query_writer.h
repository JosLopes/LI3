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
 * @file  query_writer.h
 * @brief A module responsible for outputting query results.
 *
 * @anchor query_writer_examples
 * ### Example
 *
 * The following example creates a query writer that will output unformatted objects to strings.
 * Five objects with the fields `value` and `double` are outputted.
 *
 * ```c
 * query_writer_t *writer = query_writer_create(NULL, 0);
 * if (!writer)
 *     return 1;
 *
 * for (size_t i = 0; i < 5; ++i) {
 *     query_writer_write_new_object(writer);
 *     query_writer_write_new_field(writer, "value", "%zu", i);
 *     query_writer_write_new_field(writer, "double", "%zu", 2 * i);
 * }
 * ```
 *
 * The, we can print what was outputted and delete the strings:
 *
 * ```c
 * size_t             nlines;
 * const char *const *lines = query_writer_get_lines(writer, &nlines);
 * for (size_t i = 0; i < nlines; ++i)
 *     printf("%s\n", lines[i]);
 *
 * query_writer_free(writer);
 * ```
 *
 * We should get the following as output:
 *
 * ```text
 * 0;0
 * 1;2
 * 2;4
 * 3;6
 * 4;8
 * ```
 *
 * If we had chosen a formatted output (`query_writer_create(NULL, 1)`), this would've been our
 * result:
 *
 * ```text
 * --- 1 ---
 * value: 0
 * double: 0
 *
 * --- 2 ---
 * value: 1
 * double: 2
 *
 * --- 3 ---
 * value: 2
 * double: 4
 *
 * --- 4 ---
 * value: 3
 * double: 6
 *
 * --- 5 ---
 * value: 4
 * double: 8
 * ```
 *
 * Alternatively, we can provide a output file directly to ::query_writer_create, but
 * ::query_writer_get_lines wouldn't work.
 */

#ifndef QUERY_WRITER_H
#define QUERY_WRITER_H

#include <stdio.h>

/** @brief Information about where to output query results to. */
typedef struct query_writer query_writer_t;

/**
 * @brief Creates a new place where to output query results to.
 *
 * @param out_file_path Path to file to try to open for writing. Can be `NULL`, so that query
 *                      results are outputted to internal strings.
 * @param formatted     If the output of the query should be formatted (pretty printed).
 *
 * @return A ::query_writer_t that must be deleted with ::query_writer_free. `NULL` will be returned
 *         on both IO and allocation failures.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */
query_writer_t *query_writer_create(const char *out_file_path, int formatted);

/**
 * @brief Marks that a new object will start to be written (a new flight, a new user, ...).
 * @param writer Where to write the query output to.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */
void query_writer_write_new_object(query_writer_t *writer);

/**
 * @brief Writes a field of an object to the query output.
 *
 * @param writer Where to write the query output to.
 * @param key    Name of the field being outputted.
 * @param format How to format the output (`printf` format string).
 * @param ...    Objects to be formatted accoring to @p format.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */
void query_writer_write_new_field(query_writer_t *writer, const char *key, const char *format, ...)
    __attribute__((format(printf, 3, 4)));

/**
 * @brief   Gets the lines outputted by @p writer.
 * @details Will only work if `NULL` was provided as a file path to ::query_writer_create.
 *
 * @param writer Where the query output has been written to. Cannot be `const`, as some internal
 *               buffers may need to be flushed before returning this value.
 * @param out_n  Where to output the number of lines to.
 *
 * @return The lines outputted by a query.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */
const char *const *query_writer_get_lines(query_writer_t *writer, size_t *out_n);

/**
 * @brief Frees memory allocated by ::query_writer_create.
 * @param Non-`NULL` value returned by ::query_writer_create.
 *
 * #### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */
void query_writer_free(query_writer_t *writer);

#endif
