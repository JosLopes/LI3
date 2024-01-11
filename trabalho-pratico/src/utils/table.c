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

#include <stddef.h>
#include <stdio.h>

#include "interactive_mode/ncurses_utils.h"
#include "utils/pool.h"
#include "utils/table.h"

struct table {
    pool_t    *strings;
    wchar_t ***contents_positions;
    size_t     height, width;

    int *column_widths;
};

table_t *table_create(size_t height, size_t width) {
    table_t *new_table = malloc(sizeof(table_t));

    wchar_t ***contents_positions = malloc(height * sizeof(wchar_t **));
    for (size_t i = 0; i < height; i++) {
        contents_positions[i] = malloc(width * sizeof(wchar_t *));

        for (size_t j = 0; j < width; j++)
            contents_positions[i][j] = NULL;
    }

    int *column_widths = malloc(width * sizeof(int));
    for (size_t i = 0; i < width; i++)
        column_widths[i] = 0;

    new_table->strings            = pool_create(wchar_t, TABLE_STRING_POOL_BLOCK_SIZE);
    new_table->contents_positions = contents_positions;
    new_table->height             = height;
    new_table->width              = width;
    new_table->column_widths      = column_widths;

    return new_table;
}

void table_insert(table_t *table, const wchar_t *string, size_t x, size_t y) {
    if (x != 0 || y != 0) { /* No entries for the top left corner */
        /* Calculate table measurements */
        int width = ncurses_measure_unicode_string((gunichar *) string) + 2;
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
    table_insert(table, string, x, y);
}

void __table_draw_putc_n_times(FILE    *output,
                               table_t *table,
                               char     character,
                               char     border,
                               int      starting_column) {
    for (size_t i = starting_column; i < table->width; i++) {
        putc(border, output);
        for (int j = 0; j < table->column_widths[i]; j++)
            putc(character, output);
    }
    fprintf(output, "%c\n", border);
}

void table_draw(FILE *output, table_t *table) {
    fprintf(output, " %*s", table->column_widths[0], "");
    __table_draw_putc_n_times(output, table, '-', '+', 1);

    fprintf(output, " %*s", table->column_widths[0], "");
    __table_draw_putc_n_times(output, table, ' ', '|', 1);

    fprintf(output, " %*s", table->column_widths[0], "");
    for (size_t i = 1; i < table->width; i++)
        fprintf(output, "| %*ls ", table->column_widths[i] - 2, table->contents_positions[0][i]);
    fprintf(output, "|\n");

    fprintf(output, " %*s", table->column_widths[0], "");
    __table_draw_putc_n_times(output, table, ' ', '|', 1);

    __table_draw_putc_n_times(output, table, '-', '+', 0);

    for (size_t i = 1; i < table->height; i++) {
        __table_draw_putc_n_times(output, table, ' ', '|', 0);

        for (size_t j = 0; j < table->width; j++) {
            if (table->contents_positions[i][j]) {
                fprintf(output,
                        "| %*ls ",
                        table->column_widths[j] - 2,
                        table->contents_positions[i][j]);
            } else {
                int mid_of_column = table->column_widths[j] / 2;
                fprintf(output,
                        "|%*s-%*s",
                        mid_of_column,
                        "",
                        table->column_widths[j] - mid_of_column - 1,
                        "");
            }
        }
        fprintf(output, "|\n");

        __table_draw_putc_n_times(output, table, ' ', '|', 0);
        __table_draw_putc_n_times(output, table, '-', '+', 0);
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
