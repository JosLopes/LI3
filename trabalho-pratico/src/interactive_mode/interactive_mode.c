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
 * @file  interactive_mode.c
 * @brief Implementation of methods in interactive_mode.h
 *
 * ### Examples
 * See [the header file's documentation](@ref interactive_mode_examples).
 */

/** @cond FALSE */
#define _XOPEN_SOURCE_EXTENDED
/** @endcond */

#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>

#include "dataset/dataset_loader.h"
#include "interactive_mode/activity_dataset_picker.h"
#include "interactive_mode/activity_main_menu.h"
#include "interactive_mode/activity_messagebox.h"
#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/interactive_mode.h"
#include "interactive_mode/screen_loading_dataset.h"
#include "queries/query_parser.h"

/**
 * @brief Initializes ncurses for the interactive mode.
 *
 * @retval 0 Success
 * @retval 1 Failure
 */
int __interactive_mode_init_ncurses(void) {
    setlocale(LC_ALL, "");

    /* clang-format off */

    if (initscr()            == NULL) return 1;
    if (raw()                == ERR)  return 1; /* Raw input */
    if (noecho()             == ERR)  return 1; /* Don't show input on the terminal */
    if (nl()                 == ERR)  return 1; /* Don't print carriage returns */
    if (keypad(stdscr, 1)    == ERR)  return 1; /* Let ncurses parse escape sequences */
    if (curs_set(0)          == ERR)  return 1; /* Hide the cursor */

    /* clang-format on */

    /* Limit of 10ms for ncurses to give up on finding characters for escape sequences */
    ESCDELAY = 10;

    return 0;
}

/**
 * @brief Method called when the user chooses to load a dataset in the main menu.
 * @param database Databaset to be modifed.
 */
void __interactive_mode_load_dataset(database_t **database) {
    /* Ask for dataset path */
    char *path = activity_dataset_picker_run();
    if (!path)
        return;

    /* Show that a new dataset is being loaded */
    screen_loading_dataset_render();

    /* Recreate database */
    if (*database)
        database_free(*database);

    *database = database_create();
    if (!*database) {
        activity_messagebox_run("Failed to allocate new database!");
        free(path);
        return;
    }

    /* Load new dataset */
    if (dataset_loader_load(*database, path, NULL)) {
        activity_messagebox_run("Failed to load dataset! Old data has been discarded.");
    } else {
        activity_messagebox_run("Dataset loaded successfully!");
    }

    free(path);
}

/**
 * @brief Method called when the user chooses to run a query in the main menu.
 *
 * @param query_type_list List of known query types (query definitions).
 * @param database        Database to be queried.
 */
void __interactive_mode_run_query(query_type_list_t *query_type_list, const database_t *database) {
    if (!database) {
        activity_messagebox_run("Please load a dataset first!");
        return;
    }

    gchar *query_old_str = g_strdup("");
    while (1) {
        gchar *query_str = activity_textbox_run("Input a query", query_old_str, 40);
        if (!query_str) {
            g_free(query_old_str);
            return;
        }

        query_instance_t *query_parsed = query_instance_create();
        if (query_parser_parse_string_const(query_parsed, query_str, query_type_list, NULL)) {
            g_free(query_old_str);
            query_old_str = query_str;

            query_instance_free(query_parsed, query_type_list);
            activity_messagebox_run("Failed to parse query.");
        } else {
            activity_messagebox_run("Running a query not yet supported!");

            query_instance_free(query_parsed, query_type_list);
            g_free(query_old_str);
            g_free(query_str);
            return;
        }
    }
}

/**
 * @brief Terminates ncurses when interactive mode isn't needed anymore.
 *
 * @retval 0 Success
 * @retval 1 Failure
 */
int __interactive_mode_terminate_ncurses(void) {
    return endwin() == ERR; /* Restore previous terminal mode */
}

int interactive_mode_run(void) {
    query_type_list_t *query_type_list = query_type_list_create();
    if (!query_type_list) {
        fputs("Failed to allocate query definitions!\n", stderr);
        return 1;
    }

    if (__interactive_mode_init_ncurses()) {
        query_type_list_free(query_type_list);
        return 1;
    }

    database_t *database = NULL;

    while (1) {
        activity_main_menu_chosen_option_t option = activity_main_menu_run();

        switch (option) {
            case ACTIVITY_MAIN_MENU_LOAD_DATASET:
                __interactive_mode_load_dataset(&database);
                break;
            case ACTIVITY_MAIN_MENU_RUN_QUERY:
                __interactive_mode_run_query(query_type_list, database);
                break;
            case ACTIVITY_MAIN_MENU_LEAVE:
                query_type_list_free(query_type_list);
                if (database)
                    database_free(database);

                if (__interactive_mode_terminate_ncurses())
                    return 1;
                return 0;
        }
    }
}
