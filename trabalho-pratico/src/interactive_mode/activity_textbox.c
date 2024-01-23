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
 * @brief Implementation of methods in include/interactive_mode/activity_textbox.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_textbox_examples).
 */

#include <glib.h>
#include <ncurses.h>
#include <stdlib.h>

#include "interactive_mode/activity.h"
#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/**
 * @struct activity_textbox_data_t
 * @brief  Data in a textbox TUI activity.
 *
 * @var activity_textbox_data_t::input_codepoints
 *     @brief Array of `unichar_t`s for all inputted characters.
 * @var activity_textbox_data_t::input_codepoints_width
 *     @brief Screen width of the string formed by the characters in
 *            ::activity_textbox_data_t::input_codepoints.
 * @var activity_textbox_data_t::title
 *     @brief Null-terminated UTF-32 string for the title of the textbox.
 * @var activity_textbox_data_t::text_field_width
 *     @brief Desired width of the text field in the textbox. Its real size may be smaller on
 *            smaller terminals.
 * @var activity_textbox_data_t::action_cancelled
 *     @brief Whether the user left the textbox by pressing return (`0`) or by pressing escape
 *            (`1`).
 */
typedef struct {
    GArray *input_codepoints;
    size_t  input_codepoints_width;

    unichar_t *title;

    size_t text_field_width;
    int    action_cancelled;
} activity_textbox_data_t;

/**
 * @brief   Responds to user input in a textbox activity.
 * @details Adds a valid pressed character to the inputted text.
 *
 * @param activity_data Pointer to an ::activity_textbox_data_t.
 * @param key           Key that was pressed. May be an `ncurses`' `KEY_*` value.
 * @param is_key_code   Whether the pressed key is an `ncurses`' `KEY_*` value, as opposed to a text
 *                      character.
 *
 * @retval 0 The user didn't quit the textbox; continue.
 * @retval 1 The user quit the textbox using either return or escape.
 */
int __activity_textbox_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_textbox_data_t *const textbox = activity_data;

    if (!is_key_code) {
        if (key == '\n' || key == '\x1b') {
            textbox->action_cancelled = key == '\x1b';
            return 1;
        }

        unichar_t key_int = key;
        g_array_append_val(textbox->input_codepoints, key_int);
        textbox->input_codepoints_width += ncurses_measure_character(key);
    } else if (key == KEY_BACKSPACE && textbox->input_codepoints->len > 0) {
        unichar_t last_char =
            g_array_index(textbox->input_codepoints, gunichar, textbox->input_codepoints->len - 1);
        textbox->input_codepoints_width -= ncurses_measure_character(last_char);

        g_array_set_size(textbox->input_codepoints, textbox->input_codepoints->len - 1);
    }

    return 0;
}

/**
 * @brief  Renders a textbox activity.
 * @param  activity_data Pointer to an ::activity_textbox_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_textbox_render(void *activity_data) {
    const activity_textbox_data_t *const textbox = activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 8 || window_height < 9) /* Don't attempt rendering on small windows */
        return 0;

    /* Reference diagram for positions and sizes: see header file */

    const int textbox_width  = min((size_t) window_width - 4, textbox->text_field_width + 4);
    const int textbox_height = 7;
    const int textbox_x      = (window_width - textbox_width) / 2;
    const int textbox_y      = (window_height - textbox_height) / 2;

    /* Render outer box and textbox title */
    ncurses_render_rectangle(textbox_x, textbox_y, textbox_width, textbox_height);

    size_t       title_width;
    const size_t title_max_chars =
        ncurses_prefix_from_maximum_length(textbox->title, max(textbox_width - 3, 0), &title_width);
    move(textbox_y + 1, textbox_x + (textbox_width - title_width) / 2);
    ncurses_put_wide_string(textbox->title, title_max_chars);

    /* Render text field and input text */
    const int textfield_width = textbox_width - 4;
    ncurses_render_rectangle(textbox_x + 2, textbox_y + 4, textfield_width, 1);
    move(textbox_y + 4, textbox_x + 2);

    const size_t max_text_chars =
        ncurses_suffix_from_maximum_length((const unichar_t *) textbox->input_codepoints->data,
                                           textbox->input_codepoints->len,
                                           max(textfield_width - 1, 0),
                                           NULL);
    ncurses_put_wide_string((const unichar_t *) textbox->input_codepoints->data +
                                (textbox->input_codepoints->len - max_text_chars),
                            textbox->input_codepoints->len);

    return 0;
}

/**
 * @brief Frees a textbox activity, generated by ::__activity_textbox_create.
 * @param activity_data Pointer to an ::activity_textbox_data_t.
 */
void __activity_textbox_free_data(void *activity_data) {
    activity_textbox_data_t *const textbox = activity_data;
    g_array_unref(textbox->input_codepoints);
    g_free(textbox->title);
    free(textbox);
}

/**
 * @brief   Creates an ::activity_t for a textbox.
 * @details See ::activity_textbox_run for parameter descriptions.
 * @return  An ::activity_t for a textbox, that must be deleted using ::activity_free. `NULL` is
 *          also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_textbox_create(const char *title,
                                      const char *initial_value,
                                      size_t      text_field_width) {

    activity_textbox_data_t *const activity_data = malloc(sizeof(activity_textbox_data_t));
    if (!activity_data)
        return NULL;
    activity_data->action_cancelled = 0;
    activity_data->text_field_width = text_field_width;

    /* Initialize input data */
    long             input_length;
    unichar_t *const utf32_input = g_utf8_to_ucs4_fast(initial_value, -1, &input_length);

    activity_data->input_codepoints =
        g_array_sized_new(TRUE, FALSE, sizeof(unichar_t), input_length);
    g_array_set_size(activity_data->input_codepoints, input_length);
    memcpy(activity_data->input_codepoints->data, utf32_input, input_length * sizeof(unichar_t));

    activity_data->input_codepoints_width = ncurses_measure_unicode_string(utf32_input);
    g_free(utf32_input);

    /* Initialize title */
    activity_data->title = g_utf8_to_ucs4_fast(title, -1, NULL);

    activity_t *const ret = activity_create(__activity_textbox_keypress,
                                            __activity_textbox_render,
                                            __activity_textbox_free_data,
                                            activity_data);
    if (!ret) {
        __activity_textbox_free_data(activity_data);
        return NULL;
    }
    return ret;
}

char *activity_textbox_run(const char *title, const char *initial_value, size_t text_field_width) {
    activity_t *const activity = __activity_textbox_create(title, initial_value, text_field_width);
    if (!activity)
        return NULL;

    curs_set(1);
    const activity_textbox_data_t *const textbox = activity_run(activity);
    curs_set(0);

    if (textbox) {
        if (textbox->action_cancelled) {
            activity_free(activity);
            return NULL;
        }

        gchar *const utf8 = g_ucs4_to_utf8((gunichar *) textbox->input_codepoints->data,
                                           textbox->input_codepoints->len,
                                           NULL,
                                           NULL,
                                           NULL);
        activity_free(activity);

        gchar *const normalized = g_utf8_normalize(utf8, -1, G_NORMALIZE_DEFAULT_COMPOSE);
        g_free(utf8);

        char *const ret = strdup(normalized);
        g_free(normalized); /* Make `free` the cleanup function for the return value. */

        return ret;
    }
    return NULL;
}
