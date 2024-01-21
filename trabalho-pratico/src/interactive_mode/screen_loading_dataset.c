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
 * @brief Implementation of methods in include/interactive_mode/screen_loading_dataset.h
 */

#include <glib.h>
#include <ncurses.h>

#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

void screen_loading_dataset_render(void) {
    const char *const message_utf8 = "Loading dataset. The application will become unresponsive.";
    unichar_t *const  message      = g_utf8_to_ucs4_fast(message_utf8, strlen(message_utf8), NULL);
    const size_t      text_width   = ncurses_measure_unicode_string(message);

    clear();

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    /* TODO - test dimensions */
    if (window_width < 5 || window_height < 7) { /* Don't attempt rendering on small windows */
        g_free(message);
        return;
    }

    /* Reference diagram for positions and sizes: see header file */

    const int box_width  = min((size_t) window_width - 4, text_width + 2);
    const int box_height = 3;
    const int box_x = (window_width - box_width) / 2, box_y = (window_height - box_height) / 2;

    /* Render box and message */
    ncurses_render_rectangle(box_x, box_y, box_width, box_height);

    const size_t max_chars =
        ncurses_prefix_from_maximum_length(message, max(box_width - 3, 0), NULL);
    move(box_y + 1, box_x + 1);
    ncurses_put_wide_string(message, max_chars);

    refresh();
    g_free(message);
}
