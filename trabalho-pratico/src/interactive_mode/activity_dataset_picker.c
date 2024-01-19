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
 * @file  activity_dataset_picker.c
 * @brief Implementation of methods in include/interactive_mode/activity_dataset_picker.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_dataset_picker_examples).
 */

#include <dirent.h>
#include <glib.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <unistd.h>

#include "interactive_mode/activity.h"
#include "interactive_mode/activity_dataset_picker.h"
#include "interactive_mode/activity_messagebox.h"
#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"
#include "utils/path_utils.h"

/** @brief Possible actions that the user can request when leaving a directory. */
typedef enum {
    ACTIVITY_DATASET_PICKER_ACTION_ESCAPE,     /**< Leave the dataset picker. */
    ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR,  /**< Visit the selected directory. */
    ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR, /**< Choose a directory as a dataset. */
    ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR    /**< Type the name of a new directory to visit. */
} activity_dataset_picker_action_t;

/**
 * @struct activity_dataset_picker_data_t
 * @brief  Data in a dataset picker TUI activity.
 *
 * @var activity_dataset_picker_data_t::dir_list
 *     @brief An array of `unichar_t *`, containing all sub-directories of
 *            ::activity_dataset_picker_data_t::pwd.
 * @var activity_dataset_picker_data_t::chosen_option
 *     @brief The index of the sub-directory the cursor of the directory picker is currently on.
 * @var activity_dataset_picker_data_t::action
 *     @brief Set when leaving the activity, to signal what the user desires to do next.
 * @var activity_dataset_picker_data_t::pwd
 *     @brief Current directory the user is in (UTF-32 null-terminated string).
 * @var activity_dataset_picker_data_t::pwd_len
 *     @brief Length of ::activity_dataset_picker_data_t::pwd.
 */
typedef struct {
    GPtrArray                       *dir_list;
    size_t                           chosen_option;
    activity_dataset_picker_action_t action;
    unichar_t                       *pwd;
    size_t                           pwd_len;
} activity_dataset_picker_data_t;

/**
 * @brief Responds to user input in a dataset picker activity.
 *
 * @param activity_data Pointer to an ::activity_dataset_picker_data_t.
 * @param key           Key that was pressed. May be an `ncurses`' `KEY_*` value.
 * @param is_key_code   Whether the pressed key is an `ncurses`' `KEY_*` value, as opposed to a text
 *                      character.
 *
 * @retval 0 The user didn't quit the dataset picker; continue.
 * @retval 1 The user quit the current directory by one of the ways in
 *           ::activity_dataset_picker_action_t.
 */
int __activity_dataset_picker_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_dataset_picker_data_t *const picker = activity_data;

    if (!is_key_code) {
        switch (key) {
            case '\x1b':
                picker->action = ACTIVITY_DATASET_PICKER_ACTION_ESCAPE;
                return 1;
            case '\n':
                picker->action = ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR;
                return 1;
            case 't':
            case 'T':
                picker->action = ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR;
                return 1;
            default:
                return 0;
        }
    } else {
        switch (key) {
            case KEY_UP:
                picker->chosen_option = max(0, (ssize_t) picker->chosen_option - 1);
                break;
            case KEY_DOWN:
                picker->chosen_option = min(picker->dir_list->len - 1, picker->chosen_option + 1);
                break;
            case KEY_RIGHT:
                picker->action = ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR;
                return 1;
            case KEY_LEFT:
                if (!(picker->pwd[0] == '/' && picker->pwd[1] == '\0')) {
                    /* Choose .. and leave */
                    picker->chosen_option = 0;
                    picker->action        = ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR;
                    return 1;
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

/** @brief Number of help lines rendered by ::__activity_dataset_picker_render_help_text. */
#define ACTIVITY_DATASET_PICKER_HELP_TEXT_LINE_COUNT 6

/**
 * @brief Renders help messages in the bottom of the screen.
 * @param window_width  Window width set by `getmaxyx`.
 * @param window_height Window height set by `getmaxyx`.
 */
void __activity_dataset_picker_render_help_text(int window_width, int window_height) {
    const char *const help_strings[] = {"Use \u2191 and \u2193 to cycle through directories",
                                        "Use \u2192 to visit the selected directory",
                                        "Use \u2190 to go back",
                                        "Use T to type the name of a directory",
                                        "Use ESC to leave the dataset picker",
                                        "Use Return to load the selected dataset"};

    for (size_t i = 0; i < 6; ++i) {
        const int width = min((int) ncurses_measure_string(help_strings[i]), window_width - 4);
        move(window_height - ACTIVITY_DATASET_PICKER_HELP_TEXT_LINE_COUNT - 1 + i,
             (window_width - width) / 2);
        addnstr(help_strings[i], width);
    }
}

/**
 * @brief Renders the files in the current directory and the box that contains them.
 *
 * @param picker        Information about the dataset picker.
 * @param window_width  Window width set by `getmaxyx`.
 * @param window_height Window height set by `getmaxyx`.
 */
void __activity_dataset_picker_render_file_box(const activity_dataset_picker_data_t *picker,
                                               int                                   window_width,
                                               int window_height) {

    const int box_width  = min(60, window_width - 4);
    const int box_height = window_height - ACTIVITY_DATASET_PICKER_HELP_TEXT_LINE_COUNT - 5;
    const int box_x      = (window_width - box_width) / 2;
    const int box_y      = 2;

    ncurses_render_rectangle(box_x, box_y, box_width, box_height);

    /* Render directory name */
    const size_t pwd_print_len = ncurses_suffix_from_maximum_length(picker->pwd,
                                                                    picker->pwd_len,
                                                                    max(box_width - 2, 0),
                                                                    NULL);
    move(box_y - 1, box_x + 1);
    ncurses_put_wide_string(picker->pwd + picker->pwd_len - pwd_print_len, (size_t) -1);

    /* Render files */
    const ssize_t i0_y  = box_y + (box_height) / 2 - picker->chosen_option; /* Y coord. for i = 0 */
    const size_t  i_min = max(0, (ssize_t) picker->chosen_option - (box_height) / 2);
    const size_t  complement_box_height = box_height % 2 ? (box_height / 2 + 1) : (box_height / 2);
    const size_t  i_max = min(picker->dir_list->len, picker->chosen_option + complement_box_height);

    for (size_t i = i_min; i < i_max; ++i) {
        move(i0_y + i, box_x + 1);

        if (i == picker->chosen_option) {
            /* Print white line */
            attron(A_REVERSE);
            for (int j = 0; j < box_width - 2; ++j)
                addch(' ');
            move(i0_y + i, box_x + 1);
        } else {
            attroff(A_REVERSE);
        }

        const unichar_t *const dirname = g_ptr_array_index(picker->dir_list, i);
        const size_t           drawable_chars =
            ncurses_prefix_from_maximum_length(dirname, box_width - 2, NULL);
        ncurses_put_wide_string(dirname, drawable_chars);
    }
    attroff(A_REVERSE);
}

/**
 * @brief  Renders a dataset picker activity.
 * @param  activity_data Pointer to an ::activity_dataset_picker_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_dataset_picker_render(void *activity_data) {
    const activity_dataset_picker_data_t *const picker = activity_data;

    int window_width, window_height;
    getmaxyx(stdscr, window_height, window_width);

    if (window_width < 44 || window_height < 14)
        return 0;

    __activity_dataset_picker_render_file_box(picker, window_width, window_height);
    __activity_dataset_picker_render_help_text(window_width, window_height);
    return 0;
}

/**
 * @brief Frees a textbox activity, generated by ::__activity_dataset_picker_create.
 * @param activity_data Pointer to an ::activity_dataset_picker_data_t.
 */
void __activity_dataset_picker_free_data(void *activity_data) {
    activity_dataset_picker_data_t *const picker = activity_data;
    g_ptr_array_unref(picker->dir_list);
    g_free(picker->pwd);
    free(picker);
}

/**
 * @brief   `GCompareFunc` for UTF-32 strings.
 * @details Both @p a and @p b are of type `const unichar_t **`. This is an auxiliary function for
 *          ::__activity_dataset_picker_create.
 * @return  The equivalent of `strcmp(a, b)` for UTF-32 strings.
 * */
gint __activity_dataset_picker_create_sort_strings_compare(gconstpointer a, gconstpointer b) {
    const unichar_t *const string_a = *(const unichar_t *const *) a;
    const unichar_t *const string_b = *(const unichar_t *const *) b;

    size_t i;
    for (i = 0; string_a[i] && string_b[i]; ++i)
        if (string_a[0] != string_b[0])
            break;
    return string_a[i] - string_b[i];
}

/**
 * @brief   Creates an ::activity_t for a dataset picker.
 * @details Auxiliary method for ::activity_dataset_picker_run.
 *
 * @param path Path of the directory to list.
 *
 * @return An ::activity_t for a dataset picker, that must be deleted using ::activity_free.
 *         `NULL` is also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_dataset_picker_create(const char *path) {
    activity_dataset_picker_data_t *const activity_data =
        malloc(sizeof(activity_dataset_picker_data_t));
    if (!activity_data)
        return NULL;

    glong pwd_len;
    activity_data->dir_list      = g_ptr_array_new_with_free_func(g_free);
    activity_data->chosen_option = 0;
    activity_data->action        = ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR;
    activity_data->pwd           = g_utf8_to_ucs4_fast(path, -1, &pwd_len);
    activity_data->pwd_len       = pwd_len;

    /* Load list of directories */
    DIR *const dir = opendir(path);
    if (!dir) {
        activity_messagebox_run("Error listing directory!");
        __activity_dataset_picker_free_data(activity_data);
        return NULL;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        /* Hide hidden files except "..". Always hide ".." on the root of the file system. */
        if (*ent->d_name != '.' || (strcmp(ent->d_name, "..") == 0 && strcmp(path, "/") != 0)) {
            char full_path[PATH_MAX];
            snprintf(full_path, PATH_MAX, "%s/%s", path, ent->d_name);

            struct stat statbuf; /* Only show directories */
            if (!stat(full_path, &statbuf) && S_ISDIR(statbuf.st_mode)) {
                g_ptr_array_add(activity_data->dir_list,
                                g_utf8_to_ucs4_fast(ent->d_name, -1, NULL));
            }
        }
    }
    closedir(dir);

    g_ptr_array_sort(activity_data->dir_list,
                     __activity_dataset_picker_create_sort_strings_compare);

    activity_t *const ret = activity_create(__activity_dataset_picker_keypress,
                                            __activity_dataset_picker_render,
                                            __activity_dataset_picker_free_data,
                                            activity_data);
    if (!ret) {
        __activity_dataset_picker_free_data(activity_data);
        return NULL;
    }
    return ret;
}

/**
 * @brief   Starts a textbox TUI activity to ask the user to type a directory.
 * @details Auxiliary method for ::activity_dataset_picker_run.
 *
 * @param pwd Present working directory, that will be modified to fit the text in the textbox.
 *            It's assumed that `PATH_MAX` bytes can fit in here.
 */
void __activity_dataset_picker_run_textbox(char pwd[PATH_MAX]) {
    char *const new_pwd = activity_textbox_run("Choose a directory", pwd, 60);
    if (new_pwd) {
        /* Check if directory can be opened first */
        DIR *const dir = opendir(new_pwd);
        if (!dir) {
            activity_messagebox_run("Error listing directory!");
        } else {
            path_normalize(new_pwd);
            strncpy(pwd, new_pwd, PATH_MAX);
            closedir(dir);
        }

        free(new_pwd);
    }
}

char *activity_dataset_picker_run(void) {
    char pwd[PATH_MAX];
    if (!getcwd(pwd, PATH_MAX)) {
        strcpy(pwd, "/");
    }

    while (1) {
        activity_t *const activity = __activity_dataset_picker_create(pwd);
        if (!activity)
            return NULL;

        const activity_dataset_picker_data_t *const picker = activity_run(activity);

        const unichar_t *const chosen_utf32 =
            (const unichar_t *) g_ptr_array_index(picker->dir_list, picker->chosen_option);
        gchar *const chosen = g_ucs4_to_utf8(chosen_utf32, -1, NULL, NULL, NULL);
        activity_dataset_picker_action_t action = picker->action;
        activity_free(activity);

        if (action == ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR)
            __activity_dataset_picker_run_textbox(pwd);
        else
            path_concat(pwd, chosen);

        g_free(chosen);

        if (action == ACTIVITY_DATASET_PICKER_ACTION_ESCAPE)
            return NULL;
        else if (action == ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR)
            return strdup(pwd);
    }
}
