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
 * @file  activity_paging.h
 * @brief A paginator to display the output of queries for the interactive mode.
 *
 * @anchor activity_paging_examples
 * ### Examples
 */
#ifndef ACTIVITY_PAGING_H
#define ACTIVITY_PAGING_H

#include <glib.h>

#include "interactive_mode/activity.h"

int activity_paging_run(const char **lines, size_t lines_length, size_t block_length);

#endif
