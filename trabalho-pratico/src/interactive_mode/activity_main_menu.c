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
 * @file  activity_main_menu.c
 * @brief Implementation of methods in activity_main_menu.h
 *
 * ### Examples
 * See [the header file's documentation](@ref activity_main_menu_examples).
 */

#include "interactive_mode/activity_main_menu.h"

#include "interactive_mode/activity_menu.h"

activity_main_menu_chosen_option_t activity_main_menu_run(void) {
    const char *menu_options[3] = {"Load dataset", "Run query", "Leave"};
    ssize_t     menu_return     = activity_menu_run("MAIN MENU", menu_options, 3);

    switch (menu_return) {
        case 0:
            return ACTIVITY_MAIN_MENU_LOAD_DATASET;
        case 1:
            return ACTIVITY_MAIN_MENU_RUN_QUERY;
        case 2:
            return ACTIVITY_MAIN_MENU_LEAVE;
        default:
            return ACTIVITY_MAIN_MENU_LEAVE; /* Leave on error */
    }
}
