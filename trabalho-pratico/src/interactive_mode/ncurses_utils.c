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
 * @file  ncurses_utils.c
 * @brief Implementation of methods in ncurses_utils.h
 *
 * ### Examples
 * See [the header file's documentation](@ref ncurses_utils_examples).
 */

#include <ncurses.h>

#include "interactive_mode/ncurses_utils.h"

void ncurses_render_rectangle(int x, int y, int width, int height) {
    /* Horizontal lines */
    move(y - 1, x);
    for (int i = 0; i < width; ++i)
        addch('-');

    move(y + height, x);
    for (int i = 0; i < width; ++i)
        addch('-');

    /* Vertical lines */
    for (int i = 0; i < height; ++i)
        mvaddch(y + i, x - 1, '|');

    for (int i = 0; i < height; ++i)
        mvaddch(y + i, x + width, '|');

    /* Corners */
    mvaddch(y - 1, x - 1, '+');
    mvaddch(y + height, x - 1, '+');
    mvaddch(y + height, x + width, '+');
    mvaddch(y - 1, x + width, '+');
}

int8_t ncurses_measure_character(gunichar c) {
    if (g_unichar_isprint(c)) {
        if (g_unichar_iswide(c))
            return 2;
        else if (!g_unichar_iszerowidth(c))
            return 1;
    }

    return 0;
}

size_t ncurses_measure_unicode_string(const gunichar *str) {
    size_t width = 0;

    const gunichar *iter = str;
    while (*iter) {
        width += ncurses_measure_character(*iter);
        iter++;
    }

    return width;
}

size_t ncurses_measure_string(const char *str) {
    gunichar *utf32 = g_utf8_to_ucs4_fast(str, -1, NULL);
    size_t    ret   = ncurses_measure_unicode_string(utf32);
    g_free(utf32);
    return ret;
}

size_t ncurses_prefix_from_maximum_length(gunichar *str, size_t max, size_t *width) {
    size_t acc_width = 0;

    gunichar *iter = str;
    while (*iter) {
        size_t new_width = acc_width + ncurses_measure_character(*iter);
        if (new_width > max) {
            iter++;
            break;
        }

        acc_width = new_width;
        iter++;
    }

    if (width)
        *width = acc_width;
    return (size_t) (iter - str - 1);
}

size_t ncurses_suffix_from_maximum_length(gunichar *str, size_t len, size_t max, size_t *width) {
    size_t acc_width = 0;

    gunichar *iter = str + len - 1;
    while (iter >= str) {
        size_t new_width = acc_width + ncurses_measure_character(*iter);
        if (new_width > max)
            break;

        acc_width = new_width;
        iter--;
    }

    if (width)
        *width = acc_width;
    return (size_t) (str + len - iter - 1);
}
