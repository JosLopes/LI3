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
 * @file  screen_loading_dataset.c
 * @brief Implementation of methods in screen_loading_dataset.h
 */

#include <ncurses.h>

#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

void screen_loading_dataset_render(void) {
    const wchar_t *message    = L"Loading dataset. The application will become unresponsive.";
    size_t         text_width = wcslen(message); /* No double-width characters */

    clear();

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 5 || window_height < 7) /* Don't attempt rendering on small windows */
        return;

    /* Reference diagram for positions and sizes: see header file */

    int box_width  = min((size_t) window_width - 4, text_width + 2);
    int box_height = 3;
    int box_x = (window_width - box_width) / 2, box_y = (window_height - box_height) / 2;

    /* Render box and message */
    ncurses_render_rectangle(box_x, box_y, box_width, box_height);
    size_t max_chars =
        ncurses_prefix_from_maximum_length((gunichar *) message, max(box_width - 3, 0), NULL);
    move(box_y + 1, box_x + 1);
    addnwstr(message, max_chars);

    refresh();
}
