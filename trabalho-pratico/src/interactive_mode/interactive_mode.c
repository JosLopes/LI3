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

#include "interactive_mode/activity_dataset_picker.h"
#include "interactive_mode/activity_messagebox.h"
#include "interactive_mode/activity_textbox.h"
#include "interactive_mode/interactive_mode.h"

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
 * @brief Terminates ncurses when interactive mode isn't needed anymore.
 *
 * @retval 0 Success
 * @retval 1 Failure
 */
int __interactive_mode_terminate_ncurses(void) {
    return endwin() == ERR; /* Restore previous terminal mode */
}

int interactive_mode_run(void) {
    if (__interactive_mode_init_ncurses())
        return 1;

    activity_messagebox_run("This is a message box!");

    char *dataset_path = activity_dataset_picker_run();

    if (__interactive_mode_terminate_ncurses()) {
        free(dataset_path);
        return 1;
    }

    if (dataset_path) {
        puts(dataset_path);
        free(dataset_path);
    }

    return 0;
}
