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
 * @brief Implementation of methods in activity_dataset_picker.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_dataset_picker_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <dirent.h>
#include <glib.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "interactive_mode/activity_dataset_picker.h"
#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/ncurses_utils.h"
#include "utils/int_utils.h"

/** @brief Possible actions that the user requests when leaving a directory. */
typedef enum {
    ACTIVITY_DATASET_PICKER_ACTION_ESCAPE,     /**< Leave the dataset picker */
    ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR,  /**< Visit another directory */
    ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR, /**< Choose a directory as a dataset */
    ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR    /**< Type the name of a new directory to visit */
} activity_dataset_picker_action_t;

/**
 * @struct activity_dataset_picker_data_t
 * @brief  Data in a dataset picker TUI activity.
 *
 * @var activity_dataset_picker_data_t::dir_list
 *     @brief An array of `gunichar *`, containing all sub-directories of
 *            ::activity_dataset_picker_data_t::pwd.
 * @var activity_dataset_picker_data_t::chosen_option
 *     @brief The index of the sub-directory the cursor of the directory picker is currently on.
 * @var activity_dataset_picker_data_t::action
 *     @brief Set when leaving the activity to signal what the user desires to do next.
 * @var activity_dataset_picker_data_t::pwd
 *     @brief Current directory the user is in.
 */
typedef struct {
    GPtrArray                       *dir_list;
    size_t                           chosen_option;
    activity_dataset_picker_action_t action;
    const char                      *pwd;
} activity_dataset_picker_data_t;

/**
 * @brief Responds to user input in a dataset activity.
 *
 * @param activity_data Pointer to a ::activity_dataset_picker_data_t.
 * @param key           Key that was pressed. May be an ncurses `KEY_*` value.
 * @param is_key_code   If the pressed key is not a character, but an ncurses `KEY_*` value.
 *
 * @retval 0 The user didn't quit the dataset picker; continue.
 * @retval 1 The user quit the current directory by one of the ways in
 *           ::activity_dataset_picker_action_t.
 */
int __activity_dataset_picker_keypress(void *activity_data, wint_t key, int is_key_code) {
    activity_dataset_picker_data_t *picker = (activity_dataset_picker_data_t *) activity_data;

    if (!is_key_code) {
        if (key == '\x1b') {
            picker->action = ACTIVITY_DATASET_PICKER_ACTION_ESCAPE;
            return 1;
        } else if (key == '\n') {
            picker->action = ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR;
            return 1;
        } else if (key == 't' || key == 'T') {
            picker->action = ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR;
            return 1;
        }

        return 0;
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
                if (strcmp(picker->pwd, "/") != 0) { /* Choose .. and leave */
                    picker->chosen_option = 0;
                    picker->action        = ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR;
                    return 1;
                }
                break;
        }
    }

    return 0;
}

/**
 * @brief  Renders a dataset picker activity.
 * @param  activity_data Pointer to a ::activity_dataset_picker_data_t.
 * @retval 0 Always, to continue running this activity.
 */
int __activity_dataset_picker_render(void *activity_data) {
    activity_dataset_picker_data_t *picker = (activity_dataset_picker_data_t *) activity_data;

    printw("PWD: %s PID: %d\n\n", picker->pwd, getpid());

    for (size_t i = 0; i < picker->dir_list->len; ++i) {
        if (i == picker->chosen_option)
            attron(A_REVERSE);
        else
            attroff(A_REVERSE);

        printw("%ls\n", (int32_t *) g_ptr_array_index(picker->dir_list, i));
    }

    attroff(A_REVERSE);
    return 0;
}

/**
 * @brief Frees a textbox activity, generated by ::__activity_dataset_picker_create.
 * @param activity_data Pointer to a ::activity_dataset_picker_data_t.
 */
void __activity_dataset_picker_free_data(void *activity_data) {
    activity_dataset_picker_data_t *picker = (activity_dataset_picker_data_t *) activity_data;

    for (size_t i = 0; i < picker->dir_list->len; ++i)
        g_free(g_ptr_array_index(picker->dir_list, i));
    g_ptr_array_unref(picker->dir_list);

    free(picker);
}

/**
 * @brief   `GCompareFunc` for UTF-32 strings.
 * @details Both @p a and @p b are of type `const guinchar **`. This is an auxiliary function for
 *          ::__activity_dataset_picker_create.
 * @return  The equivalent of `strcmp(a, b)` for UTF-32 strings.
 * */
gint __activity_dataset_picker_create_sort_strings(gconstpointer a, gconstpointer b) {
    const gunichar *string_a = *(const gunichar **) a;
    const gunichar *string_b = *(const gunichar **) b;

    size_t i;
    for (i = 0; string_a[i] && string_b[i]; ++i)
        if (string_a[0] != string_b[0])
            break;
    return string_a[i] - string_b[i];
}

/**
 * @brief  Creates an ::activity_t for a dataset picker.
 * @param  path Path of the directory to list.
 * @return An ::activity_t for a dataset picker, that must be deleted using ::activity_free.
 *         `NULL` is also a possibility, when an allocation failure occurs.
 */
activity_t *__activity_dataset_picker_create(const char *path) {
    activity_dataset_picker_data_t *activity_data = malloc(sizeof(activity_dataset_picker_data_t));
    if (!activity_data)
        return NULL;

    activity_data->dir_list      = g_ptr_array_new();
    activity_data->chosen_option = 0;
    activity_data->action        = ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR;
    activity_data->pwd           = path;

    /* Load list of directories */
    DIR *dir = opendir(path);
    if (!dir) {
        /* TODO - replace by message box */
        gchar *remove_this = activity_textbox_run("Error listing directory!", "Replace this with a message box", 30);
        if (remove_this)
            g_free(remove_this);

        g_ptr_array_unref(activity_data->dir_list);
        free(activity_data);
        return NULL;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (*ent->d_name != '.' || (strcmp(ent->d_name, "..") == 0 && strcmp(path, "/") != 0)) {
            char full_path[PATH_MAX];
            snprintf(full_path, PATH_MAX, "%s/%s", path, ent->d_name);

            struct stat statbuf;
            if (!stat(full_path, &statbuf) && S_ISDIR(statbuf.st_mode)) {
                g_ptr_array_add(activity_data->dir_list,
                                g_utf8_to_ucs4_fast(ent->d_name, -1, NULL));
            }
        }
    }
    closedir(dir);

    g_ptr_array_sort(activity_data->dir_list, __activity_dataset_picker_create_sort_strings);

    return activity_create(__activity_dataset_picker_keypress,
                           __activity_dataset_picker_render,
                           __activity_dataset_picker_free_data,
                           activity_data);
}

/**
 * @brief   Generates the next directory to visit, after closing a dataset picker activity.
 * @details This will handle visits to previous directories by removing text after the last slash,
 *          instead of appending `"/.."` to the end. This way, paths won't quickly grow over the
 *          operating system's limit.
 *
 * @param pwd    Previous visited directory (e.g.: `"/usr"`). The final result will be written to
 *               this address.
 * @param chosen The chosen directory to be visited next, a value relative to @p pwd (e.g.: if
 *               this parameter in `"bin"``, provided the example value of @p pwd, @p pwd will
 *               become ``"/usr/bin"``).
 */
void __activity_dataset_picker_run_generate_next_pwd(char *pwd, const gunichar *chosen) {
    if (chosen[0] != '\0' && chosen[1] != '\0' && chosen[0] == '.' && chosen[1] == '.' &&
        chosen[2] == '\0') { /* chosen == ".." */

        /* Remove all slashes from the end */
        for (size_t i = strlen(pwd) - 1; i > 0 && pwd[i] == '/'; --i)
            pwd[i] = '\0';

        ssize_t last_slash = -1;
        for (ssize_t i = 0; pwd[i]; ++i) {
            if (pwd[i] == '/')
                last_slash = i;
        }

        if (last_slash > 0)
            pwd[last_slash] = '\0';
        else if (last_slash == 0)
            strcpy(pwd, "/");
    } else {
        char pwd_tmp[PATH_MAX];
        snprintf(pwd_tmp, PATH_MAX, "%s/%ls", pwd, (int32_t *) chosen);
        strcpy(pwd, pwd_tmp);
    }
}

char *activity_dataset_picker_run(void) {
    char pwd[PATH_MAX];
    if (!getcwd(pwd, PATH_MAX)) {
        strcpy(pwd, "/");
    }

    int activity_is_textbox = 0;
    while (1) {
        if (activity_is_textbox) {
            gchar *new_pwd = activity_textbox_run("Choose a directory", pwd, 60);
            if (new_pwd) {
                 /* Check if directory can be opened first */
                DIR *dir = opendir(new_pwd);
                if (!dir) {
                    /* TODO - replace by message box */
                    gchar *remove_this = activity_textbox_run("Error listing directory!", "Replace this with a message box", 30);
                    if (remove_this)
                        g_free(remove_this);
                } else {
                    strcpy(pwd, new_pwd);
                    closedir(dir);
                }

                g_free(new_pwd);
            }

            activity_is_textbox = 0;
        } else {
            activity_t *activity = __activity_dataset_picker_create(pwd);
            if (!activity)
                return NULL;

            const activity_dataset_picker_data_t *picker =
                (const activity_dataset_picker_data_t *) activity_run(activity);

            const gunichar *chosen =
                (gunichar *) g_ptr_array_index(picker->dir_list, picker->chosen_option);

            switch (picker->action) {
                case ACTIVITY_DATASET_PICKER_ACTION_ESCAPE:
                    activity_free(activity);
                    return NULL;

                case ACTIVITY_DATASET_PICKER_ACTION_VISIT_DIR:
                    __activity_dataset_picker_run_generate_next_pwd(pwd, chosen);
                    break;

                case ACTIVITY_DATASET_PICKER_ACTION_CHOOSE_DIR:
                    __activity_dataset_picker_run_generate_next_pwd(pwd, chosen);
                    activity_free(activity);
                    return strdup(pwd);

                case ACTIVITY_DATASET_PICKER_ACTION_TYPE_DIR:
                    activity_is_textbox = 1;
                    break;
            };

            activity_free(activity);
        }
    }

    return NULL;
}
