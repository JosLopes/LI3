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
 * @file  activity.c
 * @brief Implementation of methods in activity.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <ncurses.h>
#include <stdlib.h>
#include <wchar.h>

#include "interactive_mode/activity.h"

/* TODO - docs */

/**
 * @struct activity
 * @brief A definition of a polymorphic TUI activity (through callbacks)
 *
 * @var activity::keypress_callback
 *     @brief Callback called when the user presses a key.
 * @var activity::render_callback
 *     @brief Callback called to render the activity to the curses' screen.
 * @var activity::free_data_callback
 *     @brief Callback called from ::activity_free, to clean memory used by activity::data.
 * @var activity::data
 *     @brief Data passed to all callbacks, whose type is defined by the type of this activity.
 */
struct activity {
    activity_keypress_callback_t  keypress_callback;
    activity_render_callback_t    render_callback;
    activity_free_data_callback_t free_data_callback;

    void *data;
};

activity_t *activity_create(activity_keypress_callback_t  keypress_callback,
                            activity_render_callback_t    render_callback,
                            activity_free_data_callback_t free_data_callback,
                            void                         *activity_data) {

    activity_t *activity = malloc(sizeof(struct activity));
    if (!activity)
        return NULL;

    activity->keypress_callback  = keypress_callback;
    activity->render_callback    = render_callback;
    activity->free_data_callback = free_data_callback;
    activity->data               = activity_data;

    return activity;
}

void *activity_run(activity_t *activity) {
    int cb_retval;

    cb_retval = activity->render_callback(activity->data);
    if (cb_retval)
        return activity->data;
    refresh();

    wint_t input;
    while (1) {
        int is_key_code = get_wch(&input);
        if (is_key_code == ERR)
            break;

        /*
         * Ignore Ctrl modifier. Nevertheless, Ctrl+J and Ctrl+3 still need to pass through, as
         * their codes are the same as a new line and escape.
         */
        if (input > 31 || input == '\n' || input == '\x1b' || is_key_code) {
            cb_retval = activity->keypress_callback(activity->data, input, is_key_code);
            if (cb_retval)
                return activity->data;
        }

        clear();
        cb_retval = activity->render_callback(activity->data);
        if (cb_retval)
            return activity->data;
        refresh();
    };

    return NULL;
}

void activity_free(activity_t *activity) {
    if (activity->free_data_callback)
        activity->free_data_callback(activity->data);
    free(activity);
}
