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

#include <stddef.h>
#include <stdio.h>

#include "interactive_mode/ncurses_utils.h"
#include "utils/pool.h"
#include "utils/table.h"

/**
 * @struct table
 * @brief  Information about the table to be built.
 *
 * @var table::strings
 *     @brief Allocator for the strings to be displayed in the table. 
 * @var table::contents_positions
 *     @brief A matrix that stores the strings to be displayed, their position in the matrix is
 *            the same as their position in the table.
 * @var table::height
 *     @brief Height of the table (also the height of the matrix ::table::strings).
 * @var table::width
 *     @brief Width of the table (also the height of the matrix ::table::strings).
 * @var table::column_widths
 *     @brief Stores the width of each column in the table.
 *
 * ### Examples
 * See [the header file's documentation](@ref table_examples).
 */
struct table {
    pool_t    *strings;
    wchar_t ***contents_positions;

    size_t  height, width;
    size_t *column_widths;
};

table_t *table_create(size_t height, size_t width) {
    table_t *new_table = malloc(sizeof(table_t));

    wchar_t ***contents_positions = malloc(height * sizeof(wchar_t **));
    for (size_t i = 0; i < height; i++) {
        contents_positions[i] = malloc(width * sizeof(wchar_t *));

        for (size_t j = 0; j < width; j++)
            contents_positions[i][j] = NULL;
    }

    size_t *column_widths = malloc(width * sizeof(size_t));
    for (size_t i = 0; i < width; i++)
        column_widths[i] = 0;

    new_table->strings            = pool_create(wchar_t, TABLE_STRING_POOL_BLOCK_SIZE);
    new_table->contents_positions = contents_positions;
    new_table->height             = height;
    new_table->width              = width;
    new_table->column_widths      = column_widths;

    return new_table;
}

void table_insert_wide_string(table_t *table, const wchar_t *string, size_t x, size_t y) {
    if (x != 0 || y != 0) { /* No entries for the top left corner */
        /* Calculate table measurements */
        size_t width = (size_t) ncurses_measure_unicode_string((gunichar *) string) + 2;
        if (width > table->column_widths[x])
            table->column_widths[x] = width;

        wchar_t *allocated_string =
            pool_put_items(wchar_t, table->strings, string, wcslen(string) + 1);
        (table->contents_positions)[y][x] = allocated_string;
    }
}

void table_insert_double(table_t *table, double number, size_t x, size_t y) {
    int     buffer_size = snprintf(NULL, 0, "%12.2lf", number) + 1;
    wchar_t string[buffer_size];
    swprintf(string, buffer_size, L"%12.2lf", number);
    table_insert_wide_string(table, string, x, y);
}

/**
 * @brief   Draws a table line.
 * @details The line is drawn by repeating a single character times the width of the respective
 *          column. The programmer needs to pick from which column to start and which column to end,
 *          if the end column is also the last column of the table, a '\n' is placed at the end. 
 *
 * @param output          Path to the file in which to draw the table in.
 * @param table           Table to be drawn.
 * @param ends            The character to be drawn at the extremities of each column.
 * @param filler          The character to be drawn multiple times (the same as the width of the
 *                        column) in between @p ends characters.
 * @param starting_column Column to start drawing the line.
 * @param ending_column   Column to stop drawing (this column will not be drawn), if it corresponds
 *                        with the last column of the table, a '\n' is placed at the end.
 */
void __table_draw_line(FILE    *output,
                       table_t *table,
                       char     ends,
                       char     filler,
                       size_t   starting_column,
                       size_t   ending_column) {
    for (size_t i = starting_column; i < ending_column; i++) {
        putc(ends, output);
        for (size_t j = 0; j < table->column_widths[i]; j++)
            putc(filler, output);
    }

    if (ending_column == table->width)
        fprintf(output, "%c\n", ends);
}

void table_draw(FILE *output, table_t *table) {
    /* Draw the table's header */
    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '+', '-', 1, table->width);

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '|', ' ', 1, table->width);

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    for (size_t i = 1; i < table->width; i++)
        fprintf(output,
                "| %*ls ",
                (int) table->column_widths[i] - 2,
                table->contents_positions[0][i]);
    fprintf(output, "|\n");

    __table_draw_line(output, table, ' ', ' ', 0, 1);
    __table_draw_line(output, table, '|', ' ', 1, table->width);

    __table_draw_line(output, table, '+', '-', 0, table->width);

    /* Draw the table's body */
    for (size_t i = 1; i < table->height; i++) {
        __table_draw_line(output, table, '|', ' ', 0, table->width);

        for (size_t j = 0; j < table->width; j++) {
            if (table->contents_positions[i][j]) {
                fprintf(output,
                        "| %*ls ",
                        (int) table->column_widths[j] - 2,
                        table->contents_positions[i][j]);
            } else {
                int mid_of_column = (int) table->column_widths[j] / 2;
                fprintf(output,
                        "|%*s-%*s",
                        mid_of_column,
                        "",
                        (int) table->column_widths[j] - mid_of_column - 1,
                        "");
            }
        }
        fprintf(output, "|\n");

        __table_draw_line(output, table, '|', ' ', 0, table->width);
        __table_draw_line(output, table, '+', '-', 0, table->width);
    }
}

void table_free(table_t *table) {
    pool_free(table->strings);
    for (size_t i = 0; i < table->height; i++)
        free(table->contents_positions[i]);
    free(table->contents_positions);
    free(table->column_widths);

    free(table);
}
