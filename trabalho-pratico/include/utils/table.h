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
 * @brief A table of information to be displayed to the user.
 *
 * @anchor table_examples
 * ### Example
 *
 * The following example creates a table that can be drawn to any `FILE *` (mainly oriented at
 * `stdout`).
 *
 * ```c
 * table_t *table = table_create(2, 3);
 * if (!table)
 *     return 1;
 *
 * table_insert_format(table, 1, 0, "Column 1");
 * table_insert_format(table, 2, 0, "Column 2");
 * table_insert_format(table, 0, 1, "Row 1");
 * table_insert_format(table, 1, 1, "%lf", 1.00);
 *
 * table_draw(stdout, table);
 * table_free(table);
 * ```
 *
 * The following should represent the desired output. As you can see, cells that aren't initialized
 * are filled with hyphens.
 *
 * ```text
 *         +----------+----------+
 *         |          |          |
 *         | Column 1 | Column 2 |
 *         |          |          |
 * +-------+----------+----------+
 * |       |          |          |
 * | Row 1 |     1.00 |    -     |
 * |       |          |          |
 * +-------+----------+----------+
 * ```
 */

#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

/** @brief A table to be presented to the user. */
typedef struct table table_t;

/**
 * @brief Creates a new table.
 *
 * @param wdith  Height of the table. This includes its header. Must be at least `2`.
 * @param height Width of the table.  This includes its header. Must be at least `2`.
 *
 * @return A pointer to a new table that must later be deleted with ::table_free, or `NULL` on
 *         allocation failure / invalid @p width and @p height values.
 *
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
table_t *table_create(size_t width, size_t height);

/**
 * @brief  Creates a deep copy of a table.
 * @param  table Table to be copied.
 * @return A copy of @p table that must later be deleted with ::table_free, or `NULL` on allocation
 *         failure.
 */
table_t *table_clone(const table_t *table);

/**
 * @brief Modifies the text of a cell in a table.
 *
 * @param table  Table to have the text in a given cell set.
 * @param x      Horizontal position of the cell. @p x and @p y can't simultaneously be `0`.
 * @param y      Vertical position of the cell. @p x and @p y can't simultaneously be `0`.
 * @param format How to format the cell's text (`printf` format string).
 * @param ...    Objects to be formatted accoring to @p format.
 *
 * @return 0 Success.
 * @return 1 Allocation failure or out-of-bounds position.
 *
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
int table_insert_format(table_t *table, size_t x, size_t y, const char *format, ...)
    __attribute__((format(printf, 4, 5)));

/**
 * @brief Gets the value of the string of a cell in table.
 *
 * @param table Table to get cell text from.
 * @param x     Horizontal position of the cell.
 * @param y     Vertical position of the cell.
 *
 * @return The cell's text. `NULL` can represent an out-of-bounds error or that the desired cell
 *         wasn't yet initialized.
 */
const char *table_get_cell(const table_t *table, size_t x, size_t y);

/**
 * @brief  Gets the width of a table.
 * @param  table Table to get the width from.
 * @return Width of @p table.
 */
size_t table_get_width(const table_t *table);

/**
 * @brief  Gets the height of a table.
 * @param  table Table to get the height from.
 * @return Height of @p table.
 */
size_t table_get_height(const table_t *table);

/**
 * @brief Draws the contents of a table to a file.
 *
 * @param output File stream in which to draw @p table in.
 * @param table  Table to be drawn.
 *
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_draw(FILE *output, const table_t *table);

/**
 * @brief Frees memory used by a table.
 * @param table Table to be free'd.
 *
 * #### Examples
 * See [the header file's documentation](@ref table_examples).
 */
void table_free(table_t *table);

#endif
