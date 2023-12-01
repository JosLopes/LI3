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

#include<stdlib.h>
#include<ncurses.h>

#include"interactive_mode/activity_menu.h"
#include"interactive_mode/ncurses_utils.h"
#include"utils/int_utils.h"

typedef enum {
    ACTIVITY_MENU_ACTION_ESCAPE,
    ACTIVITY_MENU_ACTION_CHOSE_OPTION,
} activity_menu_action_t;

typedef struct {
    gunichar                **options;
    gunichar                *title;

    activity_menu_action_t  action;
    int                     number_of_options;
    int                     current_option;
} activity_menu_data_t;

int __activity_menu_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    if (!is_key_code){
        if (key == '\x1b') {
            menu->action = ACTIVITY_MENU_ACTION_ESCAPE; /* Escape menu */
            return 1;
        } else if (key == '\n') {
            menu->action = ACTIVITY_MENU_ACTION_CHOSE_OPTION;
            return 1;
        }
    } else {
        switch (key){
            case KEY_UP:
                if (menu->current_option > 0)
                    menu->current_option--;

                return 0; 
            case KEY_DOWN:
                if (menu->current_option < menu->number_of_options -1)
                    menu->current_option++;

                return 0;
            case KEY_RIGHT:
                menu->action = ACTIVITY_MENU_ACTION_CHOSE_OPTION;
                return 1;
        }
    }
    return 0;
}

int __activity_menu_render(void *activity_data) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 8 || window_height < 9) /* Don't attempt rendering on small windows */
        return 0;

    attroff(A_REVERSE);

    size_t title_string_size     = ncurses_measure_unicode_string(menu->title);
    size_t max_string_size       = title_string_size;

    for (int i=0; i<menu->number_of_options; i++) {
        size_t string_size = ncurses_measure_unicode_string(menu->options[i]);        
        if (string_size > max_string_size) {
            max_string_size = string_size;
        }
    }

    int placement_y = (window_height / 2);
    int placement_x = (window_width / 2) - (max_string_size/2);

    move(max(0, placement_y -4), placement_x + (max_string_size/2) - (title_string_size/2));
    printw("%ls", menu->title);

    int menu_width  =
        min(window_width -4, (int) (max_string_size + 8));
    /* It does not support scrolling yet */
    int menu_height = min(window_height, menu->number_of_options +2); 

    ncurses_render_rectangle(placement_x -4,
                             placement_y -1,
                             menu_width,
                             menu_height);

    for (int i=0; i<menu->number_of_options; i++) {
        move(placement_y, placement_x);
        
        if (i == menu->current_option) {
            attron(A_REVERSE);
        } else {
            attroff(A_REVERSE);
        }

        printw("%ls", menu->options[i]);
        move(++placement_y, placement_x);
    }

    return 0;
}

void __activity_menu_free_data(void *activity_data) {
    activity_menu_data_t *menu = (activity_menu_data_t *) activity_data;

    for (int i=0; i<menu->number_of_options; i++) {
        g_free(menu->options[i]);
    }
    g_free(menu->options);
    free(menu);
}

activity_t *__activity_menu_create(const char *title,
                                   const char **screen_options,
                                   size_t number_of_options) {
    activity_menu_data_t *activity_data = malloc(sizeof(activity_menu_data_t));
    if (!activity_data)
        return NULL;

    activity_data->options = malloc(sizeof(gunichar *) * number_of_options);
    if (!activity_data->options) {
        free(activity_data);
        return NULL;
    }

    for (size_t i=0; i<number_of_options; i++) {
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
        ssize_t chosen_option;
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
