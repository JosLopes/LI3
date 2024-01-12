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
 * @file  table.h
 * @brief A table of contents.
 *
 * @anchor table_examples
 * ### Example
 *
 * The following example creates a table of contents that can be drawn to a file, or directly to the
 * terminal.
 *
 * ```c
 * table_t *table = table_create(2, 3);
 * if (!table)
 *     return 1;
 *
 * wchar_t column_1 [9] = L"Column 1";
 * wchar_t column_2 [9] = L"Column 2";
 * wchar_t row_1 [6]    = L"Row 1";
 * 
 * table_insert_wide_string(table, column_1, 1, 0);
 * table_insert_wide_string(table, column_2, 2, 0);
 * table_insert_wide_string(table, row_1, 0, 1);
 *
 * table_insert_double(table, 1.0, 1, 1);
 * table_insert_double(table, 134.223, 2, 1);
 *
 * table_draw(stdout, table);
 *
 * table_free(table);
 * ```
 *
 * The following should represent the desired output:
 *
 * ```text
 *         +----------+----------+
 *         |          |          |
 *         | Column 1 | Column 2 |
 *         |          |          |
 * +-------+----------+----------+
 * |       |          |          |
 * | Row 1 |     1.00 |  134.223 |
 * |       |          |          |
 * +-------+----------+----------+
 * ```
 */

#ifndef TABLE_H
#define TABLE_H

#include <wchar.h>

/** @brief Information about the table to be built. */
typedef struct table table_t;

/** @brief Size of each pool block. To store strings that will be displayed on the table. */
#define TABLE_STRING_POOL_BLOCK_SIZE 256

/**
 * @brief Creates a new table of contents.
 *
 * @param height Height of the table.
 * @param width  Width of the table.
 *
 * @return A table.
 * 
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
table_t *table_create(size_t height, size_t width);

/**
 * @brief Inserts a wide string into an existing table.
 *
 * @param table  Table to add @p string to.
 * @param string String to be added to the @p table.
 * @param x      Column of the @p table in which to insert the @p string.
 * @param y      Row of the @p table in which to insert the @p string.
 * 
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_insert_wide_string(table_t *table, const wchar_t *string, size_t x, size_t y);

/**
 * @brief   Inserts a decimal number with double precision into an existing table.
 * @details The number is displayed with a precision of 2.
 *
 * @param table  Table to add @p number to.
 * @param number Number to be added to the @p table.
 * @param x      Column of the @p table in which to insert the @p number.
 * @param y      Row of the @p table in which to insert the @p number.
 * 
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_insert_double(table_t *table, double number, size_t x, size_t y);

/**
 * @brief Draws the previously built table.
 *
 * @param output Path to the file in which to draw the @p table in.
 * @param table  Table to be drawn.
 * 
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_draw(FILE *output, table_t *table);

/**
 * @brief Free's a table.
 *
 * @param table Table to be free'd.
 * 
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_free(table_t *table);

#endif
