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
 * @file  activity.h
 * @brief A TUI activity (user interface) for interactive_mode.h
 *
 * @anchor activity_examples
 * ### Examples
 * TODO
 */

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <wchar.h>

/* TODO - docs */

typedef struct activity activity_t;

typedef int (*activity_keypress_callback_t)(void *activity_data, wint_t key, int is_key_code);

typedef int (*activity_render_callback_t)(void *activity_data);

typedef void (*activity_free_data_callback_t)(void *activity_data);

activity_t *activity_create(activity_keypress_callback_t  keypress_callback,
                            activity_render_callback_t    render_callback,
                            activity_free_data_callback_t free_data_callback,
                            void                         *activity_data);

void *activity_run(activity_t *activity);

void activity_free(activity_t *activity);

#endif
