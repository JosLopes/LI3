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
 * @file  activity_main_menu.h
 * @brief The main menu of the application.
 *
 * @anchor activity_main_menu_examples
 * ### Examples
 *
 * Running this menu is as simple as calling ::activity_main_menu_run. It will look like the
 * following on screen:
 *
 * ```text
 * +--MAIN MENU---+
 * |              |
 * | Load dataset |
 * |              |
 * | Run query    |
 * |              |
 * | Leave        |
 * |              |
 * +--------------+
 * ```
 */

#ifndef ACTIVITY_MAIN_MENU_H
#define ACTIVITY_MAIN_MENU_H

/** @brief Action the user chose to perform in the main menu. */
typedef enum {
    ACTIVITY_MAIN_MENU_LOAD_DATASET, /**< Load a dataset. */
    ACTIVITY_MAIN_MENU_RUN_QUERY,    /**< Run a query. */
    ACTIVITY_MAIN_MENU_LICENSE,      /**< Read the application's license. */
    ACTIVITY_MAIN_MENU_LEAVE         /**< Leave the application. */
} activity_main_menu_chosen_option_t;

/**
 * @brief  Runs a TUI activity for the main menu of the application.
 * @return The action the user desires to perform.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_main_menu_examples).
 */
activity_main_menu_chosen_option_t activity_main_menu_run(void);

#endif
