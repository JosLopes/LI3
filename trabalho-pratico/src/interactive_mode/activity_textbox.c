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
 * @file  activity_textvox.c
 * @brief Implementation of methods in activity_texbox.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_textbox_examples).
 */

#define _XOPEN_SOURCE_EXTENDED

#include <glib.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "interactive_mode/activity_textbox.h"

typedef struct {
    GArray *codepoints;
    char   *title;

    int action_cancelled;
} activity_textbox_data_t;

/* TODO - docs */

int __activity_textbox_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_data;

    if (!is_key_code) {
        switch (key) {
            case '\n':
                textbox->action_cancelled = 0;
                return 1;
            case '\x1b': /* Escape */
                textbox->action_cancelled = 1;
                return 1;
            default:
                break;
        }

        int32_t key_int = key;
        g_array_append_val(textbox->codepoints, key_int);
    } else {
        switch (key) {
            case KEY_BACKSPACE:
                if (textbox->codepoints->len > 0)
                    g_array_set_size(textbox->codepoints, textbox->codepoints->len - 1);
                break;

            default:
                break;
        }
    }

    return 0;
}

int __activity_textbox_render(void *activity_data) {
    activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_data;

    printw("%s\n%d\n%ls\n",
           textbox->title,
           textbox->codepoints->len,
           (int32_t *) textbox->codepoints->data);
    return 0;
}

void __activity_textbox_free_data(void *activity_data) {
    activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_data;

    g_array_unref(textbox->codepoints);
    free(textbox->title);
    free(textbox);
}

activity_t *activity_textbox_create(const char *title) {
    activity_textbox_data_t *activity_data = malloc(sizeof(activity_textbox_data_t));
    if (!activity_data)
        return NULL;

    activity_data->codepoints = g_array_new(TRUE, FALSE, sizeof(int32_t));
    activity_data->title      = strdup(title);
    if (!activity_data->title) {
        free(activity_data);
        return NULL;
    }
    activity_data->action_cancelled = 0;

    return activity_create(__activity_textbox_keypress,
                           __activity_textbox_render,
                           __activity_textbox_free_data,
                           activity_data);
}

char *activity_get_output(void *activity_run_result) {
    if (activity_run_result) {
        activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_run_result;

        if (textbox->action_cancelled)
            return NULL;

        char *ret = g_ucs4_to_utf8((gunichar *) textbox->codepoints->data,
                                   textbox->codepoints->len,
                                   NULL,
                                   NULL,
                                   NULL);
        return ret;
    }
    return NULL;
}
