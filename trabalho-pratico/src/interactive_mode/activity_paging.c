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
 * @file  activity_paging.c
 * @brief Implementation of methods in activity_paging.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_paging_examples).
 */
#include <math.h>
#include <ncurses.h>

#include "interactive_mode/activity_paging.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

typedef enum {
    ACTIVITY_PAGING_ACTION_NEXT_PAGE,
    ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE,
    ACTIVITY_PAGING_ACTION_KEEP
} activity_paging_action_t;

typedef struct {
    gunichar **lines;
    size_t     lines_length, block_length;

    size_t                   page_reference_index;
    activity_paging_action_t advance_page;
} activity_paging_data_t;

int __activity_paging_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    if (!is_key_code && key == '\x1b') {
        /* Exit menu */
        return 1;
    } else if (is_key_code) {
        switch (key) {
            case KEY_LEFT:
                paging->advance_page = ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE;
                return 0;
            case KEY_RIGHT:
                paging->advance_page = ACTIVITY_PAGING_ACTION_NEXT_PAGE;
                return 0;
            default:
                paging->advance_page = ACTIVITY_PAGING_ACTION_KEEP;
                return 0;
        }
    }

    paging->advance_page = ACTIVITY_PAGING_ACTION_KEEP;
    return 0;
}

int __activity_paging_render(void *activity_data) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    /* Reference diagram for positions and sizes: see header file */

    if ((size_t) window_height < paging->block_length + 4 || window_width < 52)
        return 0; /* Don't attempt rendering on small windows */

    int menu_height = window_height - 4;
    int menu_width  = window_width - 4;

    int menu_y = 2;
    int menu_x = 2;

    ncurses_render_rectangle(menu_x, menu_y, menu_width, menu_height);

    const char title[6] = "output";
    move(menu_y - 1, menu_x + menu_width / 2 - 3);
    printw("%s", title);

    size_t max_on_screen_lines =
        min(menu_height / paging->block_length * paging->block_length, paging->lines_length);

    size_t max_page_number = ceil((double) paging->lines_length / (double) max_on_screen_lines) - 1;
    size_t page_number     = paging->page_reference_index / max_on_screen_lines;

    if (paging->advance_page == ACTIVITY_PAGING_ACTION_NEXT_PAGE && page_number < max_page_number) {
        page_number++;
    } else if (paging->advance_page == ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE && page_number > 0) {
        page_number--;
    }
    paging->page_reference_index = page_number * max_on_screen_lines;

    size_t lines_until_end_of_page =
        min((page_number + 1) * max_on_screen_lines, paging->lines_length);

    for (size_t i = paging->page_reference_index;
         i + paging->block_length <= lines_until_end_of_page;
         i += paging->block_length) {

        for (size_t j = 0; j < paging->block_length; j++) {
            move(menu_y++, menu_x + 1);

            size_t line_max_chars = ncurses_prefix_from_maximum_length(paging->lines[i + j],
                                                                       max(menu_width - 3, 0),
                                                                       NULL);

            addnwstr((wchar_t *) paging->lines[i + j], line_max_chars);
        }
    }

    move(menu_height + 1, 3);
    if (page_number < max_page_number) {
        printw("Use -> / <- to navigate (...)");
    } else if (page_number == max_page_number) {
        printw("Use -> / <- to navigate");
    }

    move(menu_height + 1, menu_width - 14);
    printw("page %zu out of %zu", page_number, max_page_number);

    return 0;
}

void __activity_paging_free_data(void *activity_data) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    for (size_t i = 0; i < paging->lines_length; i++)
        free(paging->lines[i]);

    free(paging->lines);
    free(paging);
}

activity_t *__activity_paging_create(const char **lines, size_t lines_length, size_t block_length) {

    activity_paging_data_t *activity_data = malloc(sizeof(activity_paging_data_t));
    if (!activity_data)
        return NULL;

    activity_data->lines = malloc(lines_length * sizeof(gunichar *));
    if (!lines) {
        free(activity_data);
        return NULL;
    }

    for (size_t i = 0; i < lines_length; i++)
        activity_data->lines[i] = g_utf8_to_ucs4_fast(lines[i], -1, NULL);

    activity_data->lines_length         = lines_length;
    activity_data->block_length         = block_length;
    activity_data->page_reference_index = 0;
    activity_data->advance_page         = ACTIVITY_PAGING_ACTION_KEEP;

    return activity_create(__activity_paging_keypress,
                           __activity_paging_render,
                           __activity_paging_free_data,
                           activity_data);
}

int activity_paging_run(const char **lines, size_t lines_length, size_t block_length) {
    activity_t *activity = __activity_paging_create(lines, lines_length, block_length);
    if (!activity)
        return -1;

    activity_run(activity);

    activity_free(activity);
    return 0;
}
