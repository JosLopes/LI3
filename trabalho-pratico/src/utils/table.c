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
 * @file  table.c
 * @brief Implementation of methods in utils/table.h
 *
 * ### Examples
 * See [the header file's documentation](@ref table_examples).
 */

#include <stdio.h>

#include "interactive_mode/ncurses_utils.h"
#include "utils/string_pool.h"
#include "utils/table.h"

/**
 * @struct table
 * @brief  A table of information to be displayed to the user.
 *
 * @var table::strings
 *     @brief Allocator for the strings to be present in the table.
 * @var table::positions
 *     @brief   A matrix that associates positions to strings to be displayed.
 *     @details Position `(x, y)` can be accessed like `positions[width * y + x]`.
 * @var table::height
 *     @brief Height of the table (also the height of the matrix ::table::positions).
 * @var table::width
 *     @brief Width of the table (also the height of the matrix ::table::positions).
 * @var table::column_widths
 *     @brief Stores the width of each column in the table.
 *
 * ### Examples
 * See [the header file's documentation](@ref table_examples).
 */
struct table {
    string_pool_t *strings;
    const char   **positions;

    size_t  height, width;
    size_t *column_widths;
};

/** @brief Size of each pool block, to store strings that will be displayed on a table. */
#define TABLE_STRING_POOL_BLOCK_SIZE 2048

table_t *table_create(size_t width, size_t height) {
    if (height < 2 || width < 2)
        return NULL;

    table_t *table = malloc(sizeof(table_t));
    if (!table)
        return NULL;

    table->strings = string_pool_create(TABLE_STRING_POOL_BLOCK_SIZE);
    if (!table->strings) {
        free(table);
        return NULL;
    }

    const size_t positions_size = sizeof(char *) * width * height;
    table->positions            = malloc(positions_size);
    if (!table->positions) {
        string_pool_free(table->strings);
        free(table);
        return NULL;
    }
    memset(table->positions, 0, positions_size);

    table->column_widths = malloc(sizeof(size_t) * width);
    if (!table->column_widths) {
        free(table->positions);
        string_pool_free(table->strings);
        free(table);
        return NULL;
    }
    for (size_t i = 0; i < width; ++i)
        table->column_widths[i] = 3; /* " - " */

    table->height = height;
    table->width  = width;
    return table;
}

table_t *table_clone(const table_t *table) {
    table_t *clone = table_create(table->width, table->height);
    if (!clone)
        return NULL;
    memcpy(clone->column_widths, table->column_widths, sizeof(char *) * table->width);

    size_t line_start = 0;
    for (size_t i = 0; i < table->height; ++i) {
        for (size_t j = 0; i < table->width; ++i) {
            const char *val = table->positions[line_start + j];
            if (val) {
                const char *alloc = string_pool_put(clone->strings, val);
                if (val) {
                    clone->positions[line_start + j] = alloc;
                } else {
                    /* Allocation failure */
                    table_free(clone);
                    return NULL;
                }
            }
        }
        line_start += clone->width;
    }

    return clone;
}

int table_insert_format(table_t *table, size_t x, size_t y, const char *format, ...) {
    if (x >= table->width || y >= table->height || (x == 0 && y == 0))
        return 1;

    va_list printf_args;
    va_start(printf_args, format);

    char cell[LINE_MAX];
    vsnprintf(cell, LINE_MAX, format, printf_args);

    const size_t width = ncurses_measure_string(cell) + 2; /* + 2 -> padding */
    if (width > table->column_widths[x])
        table->column_widths[x] = width;

    const char *pool_alloc = string_pool_put(table->strings, cell);
    if (!pool_alloc) {
        va_end(printf_args);
        return 1;
    }

    table->positions[y * table->width + x] = pool_alloc;

    va_end(printf_args);
    return 0;
}

const char *table_get_cell(const table_t *table, size_t x, size_t y) {
    if (x >= table->width || y >= table->width)
        return NULL;
    return table->positions[y * table->width + x];
}

size_t table_get_width(const table_t *table) {
    return table->width;
}

size_t table_get_height(const table_t *table) {
    return table->height;
}

/**
 * @brief   Draws a division line in a table.
 * @details The line is drawn by repeating a given character (@p filler) as many times as there are
 *          characters in a column. The programmer needs to pick a start and end column. Between
 *          columns, a different character will be drawn (@p ends). If the end column is the last
 *          column of the table, a ``'\n'`` is printed.
 *
 * @param output          File stream in which to draw the table in.
 * @param table           Table to be drawn.
 * @param ends            The character to be drawn at the extremities of each column.
 * @param filler          The character to be drawn multiple times (the same as the width of the
 *                        column), in between @p ends characters.
 * @param starting_column Column in which to start drawing the line.
 * @param ending_column   Column in which to stop drawing the line (not drawn). If it is the
 *                        last column of the table, a ``'\n'`` is placed at the end.
 */
void __table_draw_line(FILE          *output,
                       const table_t *table,
                       char           ends,
                       char           filler,
                       size_t         starting_column,
                       size_t         ending_column) {

    for (size_t i = starting_column; i < ending_column; i++) {
        fputc(ends, output);
        for (size_t j = 0; j < table->column_widths[i]; j++)
            fputc(filler, output);
    }

    if (ending_column == table->width)
        fprintf(output, "%c\n", ends);
}

/**
 * @brief   Draws the text in a table cell to an output file.
 * @details The left border of the cell is also drawn.
 *
 * @param output Where to write the text to.
 * @param table  Table to get the cell text from.
 * @param x      Horizontal position of the cell.
 * @param y      Vertical position of the cell.
 */
void __table_draw_cell_text(FILE *output, const table_t *table, size_t x, size_t y) {
    const char *cell = table->positions[y * table->width + x];
    if (cell) {
        fprintf(output, "| %*s ", (int) table->column_widths[x] - 2, cell);
    } else {
        const int first_half  = table->column_widths[x] / 2;
        const int second_half = table->column_widths[x] - first_half - 1;
        fprintf(output, "|%*s-%*s", first_half, "", second_half, "");
    }
}

void table_draw(FILE *output, const table_t *table) {
    /* Draw the table's header */
    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '+', '-', 1, table->width);

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '|', ' ', 1, table->width);

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    for (size_t i = 1; i < table->width; ++i)
        __table_draw_cell_text(output, table, i, 0);
    fprintf(output, "|\n");

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '|', ' ', 1, table->width);

    __table_draw_line(output, table, '+', '-', 0, table->width);

    /* Draw the table's body */
    for (size_t i = 1; i < table->height; ++i) {
        __table_draw_line(output, table, '|', ' ', 0, table->width);

        for (size_t j = 0; j < table->width; ++j)
            __table_draw_cell_text(output, table, j, i);
        fprintf(output, "|\n");

        __table_draw_line(output, table, '|', ' ', 0, table->width);
        __table_draw_line(output, table, '+', '-', 0, table->width);
    }
}

void table_free(table_t *table) {
    string_pool_free(table->strings);
    free(table->positions);
    free(table->column_widths);
    free(table);
}
