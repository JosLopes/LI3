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
 * @file  message_box.h
 * @brief A message box that can be used to display information to the user.
 *
 * @anchor message_box_examples
 * ### Examples
 *
 * Creating a message box is as simple as calling ::message_box_run. Just provide a message,
 * and how wide you'd like the message box to be.
 *
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

int activity_messagebox_run(const char *message, size_t text_field_width);

#endif
