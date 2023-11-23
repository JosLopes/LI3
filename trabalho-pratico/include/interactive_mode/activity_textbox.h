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
 * @file  activity_textbox.h
 * @brief A textbox that can be used for user input.
 *
 * @anchor activity_textbox_examples
 * ### Examples
 * TODO - examples and docs
 */

#ifndef ACTIVITY_TEXTBOX_H
#define ACTIVITY_TEXTBOX_H

#include "interactive_mode/activity.h"

activity_t *activity_textbox_create(const char *title);

char *activity_get_output(void *activity_run_result);

#endif
