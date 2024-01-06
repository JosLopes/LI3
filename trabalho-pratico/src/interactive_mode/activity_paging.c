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

/**
 * @struct activity_paging_action_data_t
 * @brief  Stores an action performed in the paginator.
 */
typedef enum {
    ACTIVITY_PAGING_ACTION_NEXT_PAGE,     /**< @brief Move to the next page */
    ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE, /**< @brief Move to the previous page */
    ACTIVITY_PAGING_ACTION_KEEP           /**< @brief Keep on the current page */
} activity_paging_action_t;

/**
 * @struct activity_paging_data_t
 * @brief  Data in a paging TUI activity.
 *
 * @var activity_paging_data_t::lines
 *     @brief An Array of null-terminated UTF-32 strings for menu options.
 * @var activity_paging_data_t::lines_length
 *     @brief The number of lines to be displayed.
 * @var activity_paging_data_t::block_length
 *     @brief The number of lines in a block.
 * @var activity_paging_data_t::page_reference_index
 *     @brief The line where the current page being displayed starts.
 * @var activity_paging_data_t::change_page
 *     @brief An action to change, or keep, the current page.
 */
typedef struct {
    gunichar **lines;
    size_t     lines_length, block_length;

    size_t                   page_reference_index;
    activity_paging_action_t change_page;
} activity_paging_data_t;

/**
 * @brief   Responds to user input in a paging activity.
 * @details Handles user input to navigate through the pages of outputs, if necessary.
 *
 * @param activity_data Pointer to a ::activity_paging_data_t.
 * @param key           Key that was pressed. May be an ncurses `KEY_*` value.
 * @param is_key_code   If the pressed key is not a character, but an ncurses `KEY_*` value.
 *
 * @retval 0 The user didn't quit or performed an action; continue.
 * @retval 1 The user quit the menu using `\x1b` (Escape key).
 */
int __activity_paging_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    if (!is_key_code && key == '\x1b') {
        /* Exit menu */
        return 1;
    } else if (is_key_code) {
        switch (key) {
            case KEY_LEFT:
                paging->change_page = ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE;
                return 0;
            case KEY_RIGHT:
                paging->change_page = ACTIVITY_PAGING_ACTION_NEXT_PAGE;
                return 0;
            default:
                paging->change_page = ACTIVITY_PAGING_ACTION_KEEP;
                return 0;
        }
    }

    paging->change_page = ACTIVITY_PAGING_ACTION_KEEP;
    return 0;
}

/**
 * @brief  Renders a paging activity.
 * @param  activity_data Pointer to a ::activity_paging_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_paging_render(void *activity_data) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    /* Reference diagram for positions and sizes: see header file */

    if ((size_t) window_height < paging->block_length + 4 || window_width < 56)
        return 0; /* Don't attempt rendering on small windows */

    int menu_height = window_height - 4;
    int menu_width  = window_width - 4;

    const int menu_y = 2;
    const int menu_x = 2;

    ncurses_render_rectangle(menu_x, menu_y, menu_width, menu_height);

    /* Print the title */
    move(menu_y - 1, menu_x + (menu_width - strlen("Query output")) / 2);
    printw("%s", "Query output");

    size_t max_on_screen_lines =
        min(menu_height / paging->block_length * paging->block_length, paging->lines_length);

    size_t max_page_number = ceil((double) paging->lines_length / (double) max_on_screen_lines) - 1;
    size_t page_number     = paging->page_reference_index / max_on_screen_lines;

    /*
     * Handles page changes. This is done here because only the renderer known about screen
     * dimensions
     */
    if (paging->change_page == ACTIVITY_PAGING_ACTION_NEXT_PAGE && page_number < max_page_number) {
        page_number++;
    } else if (paging->change_page == ACTIVITY_PAGING_ACTION_PREVIOUS_PAGE && page_number > 0) {
        page_number--;
    }
    paging->page_reference_index = page_number * max_on_screen_lines;

    /* Prints paging information relevant to the user */
    if (max_page_number != 0) {
        move(menu_y + menu_height - 1, menu_x + 1);
        if (page_number <= max_page_number) {
            printw("Use the left and right arrows to navigate");
        }

        char   ratio[256];
        size_t len = snprintf(ratio, 256, "%zu / %zu", page_number + 1, max_page_number + 1);
        move(menu_y + menu_height - 1, menu_x + menu_width - len - 1);
        printw("%s", ratio);
    }

    size_t lines_until_end_of_current_page =
        min((page_number + 1) * max_on_screen_lines, paging->lines_length);

    /* Prints the blocks of lines that fit in the current page */
    int text_y = menu_y;
    for (size_t i = paging->page_reference_index;
         i + paging->block_length <= lines_until_end_of_current_page;
         i += paging->block_length) {

        for (size_t j = 0; j < paging->block_length; j++) {
            move(text_y, menu_x + 1);
            text_y++;

            if (i + j >= paging->lines_length)
                return 0; /* Reached end of text */

            size_t line_max_chars = ncurses_prefix_from_maximum_length(paging->lines[i + j],
                                                                       max(menu_width - 3, 0),
                                                                       NULL);

            addnwstr((wchar_t *) paging->lines[i + j], line_max_chars);
        }
    }

    return 0;
}

/**
 * @brief Frees a paging activity, generated by ::__activity_paging_create.
 * @param activity_data Pointer to a ::activity_paging_data_t.
 */
void __activity_paging_free_data(void *activity_data) {
    activity_paging_data_t *paging = (activity_paging_data_t *) activity_data;

    for (size_t i = 0; i < paging->lines_length; i++)
        free(paging->lines[i]);

    free(paging->lines);
    free(paging);
}

/**
 * @brief   Creates an ::activity_t for a paginator.
 * @details See ::activity_paging_run for parameter information.
 * @return  An ::activity_t for a paginator, that must be deleted using ::activity_free. `NULL` is
 *          also a possibility, when an allocation failure occurs.
 */
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
    activity_data->change_page          = ACTIVITY_PAGING_ACTION_KEEP;

    return activity_create(__activity_paging_keypress,
                           __activity_paging_render,
                           __activity_paging_free_data,
                           activity_data);
}

int activity_paging_run(const char **lines, size_t lines_length, size_t block_length) {
    activity_t *activity = __activity_paging_create(lines, lines_length, block_length);
    if (!activity)
        return 1;

    activity_run(activity);

    activity_free(activity);
    return 0;
}
