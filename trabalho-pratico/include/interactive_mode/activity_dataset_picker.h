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
 * @file  activity_dataset_picker.h
 * @brief A file manager where the user can pick the directory where their dataset is.
 *
 * @anchor activity_dataset_picker_examples
 * ### Examples
 *
 * Creating a dataset is as simple as calling ::activity_dataset_picker_run. It will return the
 * path to the selected dataset, or `NULL` if the user didn't choose one.
 *
 * The dataset picker will look like the following on screen:
 *
 * ```text
 * +-/dir/path---+
 * |             |
 * | Sub dir 1   |
 * | Sub dir 2   |
 * | SELECTED    |
 * | Sub dir 3   |
 * | Sub dir 4   |
 * | Sub dir 5   |
 * |             |
 * +-------------+
 *
 *   Usage  tips
 * ```
 */

#ifndef ACTIVITY_DATASET_PICKER_H
#define ACTIVITY_DATASET_PICKER_H

#include "interactive_mode/activity.h"

/**
 * @brief Runs a TUI activity for a dataset picker.
 * @return The path to the dataset directory on success, `NULL` on failure or user cancellation. If
 *         non-`NULL`, `free` must be called for the returned value.
 *
 * #### Examples
 * See [the header file's documentation](@ref activity_dataset_picker_examples).
 */
char *activity_dataset_picker_run(void);

#endif
