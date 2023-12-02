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
 * @file  message_box.c
 * @brief Implementation of methods in message_box.h
 *
 * ### Examples
 * See [the header file's documentation](@ref message_box_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "interactive_mode/activity_messagebox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

typedef struct {
    gunichar *message;

    size_t text_field_width;
    int    action_cancelled;
} activity_messagebox_data_t;

int __activity_messagebox_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) activity_data;

    if (!is_key_code) {
        switch (key) {
            case '\n':
                messagebox->action_cancelled = 1;
                return 1;
            case '\x1b': /* Escape */
                messagebox->action_cancelled = 1;
                return 1;
            default:
                break;
        }
    }

    return 0;
}

int __activity_messagebox_render(void *activity_data) {
    activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 8 || window_height < 9) /* Don't attempt rendering on small windows */
        return 0;

    /* Reference diagram for positions and sizes: see header file */

    int messagebox_width  = min((size_t) window_width - 4, messagebox->text_field_width + 4);
    int messagebox_height = 7;

    int messagebox_x = (window_width - messagebox_width) / 2;
    int messagebox_y = (window_height - messagebox_height) / 2;

    /* Render outer box and message */
    ncurses_render_rectangle(messagebox_x, messagebox_y, messagebox_width, messagebox_height);

    size_t message_width,
        message_max_chars = ncurses_prefix_from_maximum_length(messagebox->message,
                                                               max(messagebox_width - 3, 0),
                                                               &message_width);

    int32_t deleted                            = messagebox->message[message_max_chars + 1];
    messagebox->message[message_max_chars + 1] = '\0';

    move(messagebox_y + (messagebox_height / 2), messagebox_x + (messagebox_width - message_width) / 2);
    printw("%ls", (wchar_t *) messagebox->message);

    messagebox->message[message_max_chars + 1] = deleted;

    return 0;
}

void __activity_messagebox_free_data(void *activity_data) {
    activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) activity_data;

    g_free(messagebox->message);
    free(messagebox);
}

activity_t *__activity_messagebox_create(const char *message, size_t text_field_width) {
    activity_messagebox_data_t *activity_data = malloc(sizeof(activity_messagebox_data_t));
    if (!activity_data)
        return NULL;

    /* Initialize message */
    activity_data->message = g_utf8_to_ucs4_fast(message, -1, NULL);

    /* Initialize */
    activity_data->action_cancelled = 0;
    activity_data->text_field_width = text_field_width;

    return activity_create(__activity_messagebox_keypress,
                           __activity_messagebox_render,
                           __activity_messagebox_free_data,
                           activity_data);
}

int activity_messagebox_run(const char *message, size_t text_field_width) {
    activity_t *activity = __activity_messagebox_create(message, text_field_width);
    if (!activity)
        return 1;

    curs_set(1);
    void *run_result = activity_run(activity);
    curs_set(0);

    if (run_result) {
        activity_messagebox_data_t *messagebox = (activity_messagebox_data_t *) run_result;

        if (messagebox->action_cancelled) {
            activity_free(activity);
            return 0;
        }
    }

    return 1;
}
