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
 * @file  activity_messagebox.h
 * @brief A message box that can be used to display information to the user.
 *
 * @anchor activity_messagebox_examples
 * ### Examples
 *
 * Creating a message box is as simple as calling ::activity_messagebox_run: just provide a message!
 * The message box will look like the following on screen:
 *
 * ```text
 * +-------------+
 * |             |
 * |   MESSAGE   |
 * |             |
 * +-------------+
 * ```
 */

#ifndef ACTIVITY_MESSAGEBOX_H
#define ACTIVITY_MESSAGEBOX_H

#include "interactive_mode/activity.h"

/**
 * @brief Runs a TUI activity for a message box.
 *
 * @param message The message that will be shown on the screen. Must be a single line of text.
 *
 * @retval 0 Success
 * @retval 1 Allocation failure
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_messagebox_examples).
 */
int activity_messagebox_run(const char *message);

#endif
