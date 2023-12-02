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
 * @file  activity_menu.c
 * @brief Implementation of methods in activity_menu.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_menu_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <ncurses.h>
#include <stdlib.h>

#include "interactive_mode/activity_menu.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/**
 * @brief Indicates the action performed by the user.
 */
typedef enum {
    ACTIVITY_MENU_ACTION_ESCAPE,       /**< @brief Escape the menu without choosing an option. */
    ACTIVITY_MENU_ACTION_CHOSE_OPTION, /**< @brief Chose the highlighted option. */
} activity_menu_action_t;

/**
 * @struct activity_menu_data_t
 * @brief  Data in a menu TUI activity.
 *
 * @var activity_menu_data_t::options
 *     @brief An Array of null-terminated UTF-32 strings for menu options.
 * @var activity_menu_data_t::title
 *     @brief Null-terminated UTF-32 string for the title of the menu.
 * @var activity_menu_data_t::action
 *     @brief Action performed by the user inside the menu.
 * @var activity_menu_data_t::number_of_options
 *     @brief Number of options to display on the menu.
 * @var activity_menu_data_t::current_option
 *     @brief Current highlighted option.
 */
typedef struct {
    gunichar **options;
    gunichar  *title;

    activity_menu_action_t action;
    int                    number_of_options;
    int                    current_option;
} activity_menu_data_t;

/**
 * @brief   Responds to user input in a menu activity.
 * @details Handles user input to navigate the menu and select one of multiple possible options.
 *
 * @param activity_data Pointer to a ::activity_menu_data_t.
 * @param key           Key that was pressed. May be an ncurses `KEY_*` value.
 * @param is_key_code   If the pressed key is not a character, but an ncurses `KEY_*` value.
 *
 * @retval 0 The user didn't quit the menu; continue.
 * @retval 1 The user quit the menu using either `\n` (Enter key), `\x1b` (Escape key), or
 *           `KEY_RIGHT`.
 */
int __activity_menu_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    if (!is_key_code) {
        if (key == '\x1b') {
            menu->action = ACTIVITY_MENU_ACTION_ESCAPE; /* Escape menu */
            return 1;
        } else if (key == '\n') {
            menu->action = ACTIVITY_MENU_ACTION_CHOSE_OPTION;
            return 1;
        }
    } else {
        switch (key) {
            case KEY_UP:
                if (menu->current_option > 0)
                    menu->current_option--;

                return 0;
            case KEY_DOWN:
                if (menu->current_option < menu->number_of_options - 1)
                    menu->current_option++;

                return 0;
            case KEY_RIGHT:
                menu->action = ACTIVITY_MENU_ACTION_CHOSE_OPTION;
                return 1;
        }
    }
    return 0;
}

/**
 * @brief Renders a menu activity.
 * @param activity_data Pointer to a ::activity_menu_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_menu_render(void *activity_data) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 8 || window_height < 9) /* Don't attempt rendering on small windows */
        return 0;

    /* Reference diagram for positions and sizes: see header file */

    size_t title_max_chars =
        ncurses_prefix_from_maximum_length(menu->title, max(window_width - 2, 0), NULL) + 1;
    size_t max_string_size = title_max_chars;

    for (int i = 0; i < menu->number_of_options; i++) {
        size_t string_size = ncurses_measure_unicode_string(menu->options[i]);
        if (string_size > max_string_size) {
            max_string_size = string_size;
        }
    }

    int menu_width  = min(window_width - 2, (int) (max_string_size + 8));
    int menu_height = min(window_height - 3, menu->number_of_options + 2);

    int placement_y = max(2, (window_height - menu_height) / 2);
    int placement_x = (window_width - max_string_size) / 2;

    /* Render title */
    attroff(A_REVERSE);
    move(max(0, placement_y - 4), placement_x + (max_string_size - title_max_chars) / 2);
    addnwstr((wchar_t *) menu->title, title_max_chars);

    /* Render menu box */
    ncurses_render_rectangle(max(placement_x - 4, 1), placement_y, menu_width, menu_height);

    /* Set parameters to display the correct options, dependent on display size */
    int max_on_screen_options = min(menu_height - 2, menu->number_of_options);
    int scroll_menu_number    = menu->current_option / max_on_screen_options;
    int i                     = scroll_menu_number * max_on_screen_options;
    int set_of_options =
        min(max_on_screen_options * scroll_menu_number + menu_height - 2, menu->number_of_options);

    /* Render options */
    placement_y++;
    for (; i < set_of_options; i++) {
        move(placement_y, max(placement_x, 1));

        if (i == menu->current_option) {
            attron(A_REVERSE);
        } else {
            attroff(A_REVERSE);
        }

        size_t option_max_chars =
            ncurses_prefix_from_maximum_length(menu->options[i], menu_width - 1, NULL) + 1;
        addnwstr((wchar_t *) menu->options[i], option_max_chars);
        move(++placement_y, max(placement_x, 1));
    }

    return 0;
}

/**
 * @brief Frees a menu activity, generated by ::__activity_menu_create.
 * @param activity_data Pointer to a ::activity_menu_data_t.
 */
void __activity_menu_free_data(void *activity_data) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    for (int i = 0; i < menu->number_of_options; i++) {
        g_free(menu->options[i]);
    }
    g_free(menu->title);
    g_free(menu->options);
    free(menu);
}

/**
 * @brief   Creates an ::activity_t for a menu.
 * @details See ::activity_menu_run for parameter information.
 * @return  An ::activity_t for a menu, that must be deleted using ::activity_free. `NULL` is
 *          also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_menu_create(const char  *title,
                                   const char **screen_options,
                                   size_t       number_of_options) {
    activity_menu_data_t *activity_data = malloc(sizeof(activity_menu_data_t));
    if (!activity_data)
        return NULL;

    activity_data->options = malloc(sizeof(gunichar *) * number_of_options);
    if (!activity_data->options) {
        free(activity_data);
        return NULL;
    }

    for (size_t i = 0; i < number_of_options; i++) {
        activity_data->options[i] = g_utf8_to_ucs4_fast(screen_options[i], -1, NULL);
    }

    activity_data->title = g_utf8_to_ucs4_fast(title, -1, NULL);

    activity_data->action            = ACTIVITY_MENU_ACTION_CHOSE_OPTION;
    activity_data->number_of_options = number_of_options;
    activity_data->current_option    = 0;

    return activity_create(__activity_menu_keypress,
                           __activity_menu_render,
                           __activity_menu_free_data,
                           activity_data);
}

int activity_menu_run(const char *title, const char **screen_options, size_t number_of_options) {
    activity_t *activity = __activity_menu_create(title, screen_options, number_of_options);
    if (!activity)
        return -1;

    void *run_result = activity_run(activity);

    if (run_result) {
        ssize_t               chosen_option;
        activity_menu_data_t *menu = (activity_menu_data_t *) run_result;

        if (menu->action == ACTIVITY_MENU_ACTION_CHOSE_OPTION) {
            chosen_option = menu->current_option;
        } else {
            chosen_option = -1;
        }

        activity_free(activity);
        return chosen_option;
    }

    return -1;
}
