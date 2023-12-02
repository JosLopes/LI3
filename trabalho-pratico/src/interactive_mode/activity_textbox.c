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
 * @file  activity_textbox.c
 * @brief Implementation of methods in activity_textbox.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_textbox_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/**
 * @struct activity_textbox_data_t
 * @brief  Data in a textbox TUI activity.
 *
 * @var activity_textbox_data_t::input_codepoints
 *     @brief Array of `gunichar`s for all inputted characters.
 * @var activity_textbox_data_t::input_codepoints_width
 *     @brief Screen width of the string formed by the characters in
 *            ::activity_textbox_data_t::input_codepoints.
 * @var activity_textbox_data_t::title
 *     @brief Null-terminated UTF-32 string for the title of the textbox.
 * @var activity_textbox_data_t::text_field_width
 *     @brief Desired width of the text field in the textbox. The real size may be smaller on
 *            smaller terminals.
 * @var activity_textbox_data_t::action_cancelled
 *     @brief Whether the user left the textbox by pressing return (`0`) or by pressing escape
 *            (`1`).
 */
typedef struct {
    GArray *input_codepoints;
    size_t  input_codepoints_width;

    gunichar *title;

    size_t text_field_width;
    int    action_cancelled;
} activity_textbox_data_t;

/**
 * @brief   Responds to user input in a textbox activity.
 * @details Adds a valid pressed character to the inputted text.
 *
 * @param activity_data Pointer to a ::activity_textbox_data_t.
 * @param key           Key that was pressed. May be an ncurses `KEY_*` value.
 * @param is_key_code   If the pressed key is not a character, but an ncurses `KEY_*` value.
 *
 * @retval 0 The user didn't quit the textbox; continue.
 * @retval 1 The user quit the textbox using either return or escape.
 */
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

        gunichar key_int = key;
        g_array_append_val(textbox->input_codepoints, key_int);
        textbox->input_codepoints_width += ncurses_measure_character(key);
    } else if (key == KEY_BACKSPACE && textbox->input_codepoints->len > 0) {
        gunichar last_char =
            g_array_index(textbox->input_codepoints, gunichar, textbox->input_codepoints->len - 1);
        textbox->input_codepoints_width -= ncurses_measure_character(last_char);

        g_array_set_size(textbox->input_codepoints, textbox->input_codepoints->len - 1);
    }

    return 0;
}

/**
 * @brief Renders a textbox activity.
 * @param activity_data Pointer to a ::activity_textbox_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_textbox_render(void *activity_data) {
    activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 8 || window_height < 9) /* Don't attempt rendering on small windows */
        return 0;

    /* Reference diagram for positions and sizes: see header file */

    int textbox_width  = min((size_t) window_width - 4, textbox->text_field_width + 4);
    int textbox_height = 7;

    int textbox_x = (window_width - textbox_width) / 2;
    int textbox_y = (window_height - textbox_height) / 2;

    /* Render outer box and textbox title */
    ncurses_render_rectangle(textbox_x, textbox_y, textbox_width, textbox_height);

    size_t title_width,
        title_max_chars = ncurses_prefix_from_maximum_length(textbox->title,
                                                             max(textbox_width - 3, 0),
                                                             &title_width);
    move(textbox_y + 1, textbox_x + (textbox_width - title_width) / 2);
    addnwstr((wchar_t *) textbox->title, title_max_chars);

    /* Render text field and input text */
    int textfield_width = textbox_width - 4;
    ncurses_render_rectangle(textbox_x + 2, textbox_y + 4, textfield_width, 1);
    move(textbox_y + 4, textbox_x + 2);

    size_t max_text_chars =
        ncurses_suffix_from_maximum_length((gunichar *) textbox->input_codepoints->data,
                                           textbox->input_codepoints->len,
                                           max(textfield_width - 1, 0),
                                           NULL);
    addwstr((wchar_t *) textbox->input_codepoints->data +
            (textbox->input_codepoints->len - max_text_chars));

    return 0;
}

/**
 * @brief Frees a textbox activity, generated by ::__activity_textbox_create.
 * @param activity_data Pointer to a ::activity_textbox_data_t.
 */
void __activity_textbox_free_data(void *activity_data) {
    activity_textbox_data_t *textbox = (activity_textbox_data_t *) activity_data;

    g_array_unref(textbox->input_codepoints);
    g_free(textbox->title);
    free(textbox);
}

/**
 * @brief   Creates an ::activity_t for a textbox.
 * @details See ::activity_textbox_run for parameter information.
 * @return  An ::activity_t for a textbox, that must be deleted using ::activity_free. `NULL` is
 *          also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_textbox_create(const char *title,
                                      const char *initial_value,
                                      size_t      text_field_width) {

    activity_textbox_data_t *activity_data = malloc(sizeof(activity_textbox_data_t));
    if (!activity_data)
        return NULL;

    /* Initialize input data */
    long      input_length;
    gunichar *utf32_input = g_utf8_to_ucs4_fast(initial_value, -1, &input_length);

    activity_data->input_codepoints =
        g_array_sized_new(TRUE, FALSE, sizeof(gunichar), input_length);
    memcpy(activity_data->input_codepoints->data, utf32_input, input_length * sizeof(gunichar));
    g_array_set_size(activity_data->input_codepoints, input_length);

    activity_data->input_codepoints_width = ncurses_measure_unicode_string(utf32_input);
    g_free(utf32_input);

    /* Initialize title */
    activity_data->title = g_utf8_to_ucs4_fast(title, -1, NULL);

    /* Initialize */
    activity_data->action_cancelled = 0;
    activity_data->text_field_width = text_field_width;

    return activity_create(__activity_textbox_keypress,
                           __activity_textbox_render,
                           __activity_textbox_free_data,
                           activity_data);
}

gchar *activity_textbox_run(const char *title, const char *initial_value, size_t text_field_width) {
    activity_t *activity = __activity_textbox_create(title, initial_value, text_field_width);
    if (!activity)
        return NULL;

    curs_set(1);
    void *run_result = activity_run(activity);
    curs_set(0);

    if (run_result) {
        activity_textbox_data_t *textbox = (activity_textbox_data_t *) run_result;

        if (textbox->action_cancelled) {
            activity_free(activity);
            return NULL;
        }

        gchar *utf8 = g_ucs4_to_utf8((gunichar *) textbox->input_codepoints->data,
                                     textbox->input_codepoints->len,
                                     NULL,
                                     NULL,
                                     NULL);

        gchar *normalized = g_utf8_normalize(utf8, -1, G_NORMALIZE_DEFAULT_COMPOSE);

        g_free(utf8);
        activity_free(activity);
        return normalized;
    }

    return NULL;
}
